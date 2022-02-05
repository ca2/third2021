#include "framework.h"

i32 iMp3PacketSize = 1024;

namespace rtp
{

   file::file()
   {
      m_uiTimeStamp        = 0;
      jittcomp             = 1000;
      adapt                = FALSE;
      clockslide           = 0;
      m_dwSynchSource      = 1;
      m_millisTimeout          = 120 * 1000;
      format               = 0;
      jitter               = 0;
      m_iRemotePort        = -1;
      m_iListenPort        = -1;
      m_psession           = nullptr;
      m_pprofile           = nullptr;
      m_bStreamReceived    = false;
   }

   file::~file()
   {
   }

   bool file::rx_open(const char * pszAddress, i32 iPort)
   {

      m_strListenAddress = pszAddress;
      m_iListenPort = iPort;
      if(m_set.has_property("--noadapt"))
         adapt=FALSE;
      if(m_set.has_property("--format"))
      {
         adapt=FALSE;
         if(m_set["--format"] == "mulaw")
         {
            //format=MULAW;
         }
         else if(m_set["--format"] == "alaw")
         {
            //format=ALAW;
         }
         else
         {
            //TRACE("Unsupported format %s\n",argv[i]);
            return false;
         }
      }
      if(m_set.has_property("--with-jitter"))
      {
         jittcomp=m_set["--with-jitter"];
         TRACE("Using a jitter buffer of %i milliseconds.\n",jittcomp);
      }

      //jittcomp=100;
      //adapt = true;

      ortp_init();
      ortp_scheduler_init();
      ortp_set_log_level_mask(ORTP_DEBUG|ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
      m_psession = new ::rtp::session(RTP_SESSION_RECVONLY);
      m_psession->set_scheduling_mode(1);
      m_psession->set_blocking_mode(1);
      rtp_session_set_local_addr(m_psession->m_psession, m_strListenAddress, m_iListenPort);
      //rtp_session_set_connected_mode(m_psession->m_psession,TRUE);
      rtp_session_set_connected_mode(m_psession->m_psession,FALSE);
      rtp_session_set_symmetric_rtp(m_psession->m_psession,TRUE);
      rtp_session_enable_adaptive_jitter_compensation(m_psession->m_psession,adapt);
      rtp_session_set_jitter_compensation(m_psession->m_psession,jittcomp);
      rtp_session_set_payload_type(m_psession->m_psession,0);
      //rtp_session_signal_connect(m_psession->m_psession,"ssrc_changed",(RtpCallback)ssrc_cb,0);
      rtp_session_signal_connect(m_psession->m_psession,"ssrc_changed",(RtpCallback)rtp_session_reset,0);
      m_iHaveMore = 1;
      return true;
   }

/*
 The algorithm computes two values:
	slide: an average of difference between the expected and the socket-received timestamp
	jitter: an average of the absolute value of the difference between socket-received timestamp and slide.
	slide is used to make clock-slide detection and correction.
	jitter is added to the initial jitt_comp_time value. It compensates bursty packets arrival (packets
	not arriving at regular interval ).
*/
//void jitter_control_new_packet(JitterControl *ctl, u32 packet_ts, u32 cur_str_ts, i32 * slide, i32 *safe_delay)
   bool file::tx_open(const char * pszAddress, i32 iPort)
   {
      m_strRemoteAddress = pszAddress;
      m_iRemotePort = iPort;
      clockslide = 0;
      if(m_set.has_property("--with-clockslide"))
      {
         clockslide=atoi(m_set["--with-clockslide"]);
         ortp_message("Using clockslide of %i milisecond every 50 packets.",clockslide);
      }
      jitter = 0;
      if(m_set.has_property("--with-jitter"))
      {
         ortp_message("Jitter will be added to outgoing stream.");
         jitter=atoi(m_set["--with-jitter"]);
      }

      ortp_init();
      ortp_scheduler_init();
      ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
      m_psession = new ::rtp::session(RTP_SESSION_SENDONLY);
      if(m_psession->m_psession == nullptr)
      {
         return false;
      }

      rtp_session_set_scheduling_mode(m_psession->m_psession,1);
      rtp_session_set_blocking_mode(m_psession->m_psession,1);
      //rtp_session_set_connected_mode(m_psession->m_psession,TRUE);
      rtp_session_set_connected_mode(m_psession->m_psession,FALSE);
      rtp_session_enable_adaptive_jitter_compensation(m_psession->m_psession,TRUE);
      rtp_session_set_jitter_compensation(m_psession->m_psession,1000);

      if(rtp_session_set_remote_addr(m_psession->m_psession,m_strRemoteAddress,m_iRemotePort) != 0)
         goto cleanup;
      rtp_session_set_payload_type(m_psession->m_psession,0);

  //    if(jitter > 0)
    //  {
      //   rtp_session_set_jitter_compensation(m_psession->m_psession, jitter);
      //}

      if(m_dwSynchSource != 0)
      {
         TRACE("using SSRC=%i.\n",m_dwSynchSource);
         rtp_session_set_ssrc(m_psession->m_psession,m_dwSynchSource);
      }
      return true;
cleanup:
      rtp_session_destroy(m_psession->m_psession);
      ortp_exit();
      ortp_global_stats_display();

      return false;
   }



   memsize file::read(void *lpBuf, memsize nCount)
   {
      memsize uRead = 0;
auto tickStart = ::millis::now();
      memory mem;
      mem.set_size(nCount);
//      u32 uiStart = m_uiTimeStamp;
      while(m_memfile.get_size() <= 0)
      {
         m_iHaveMore = 1;
		   while (m_iHaveMore)
         {

            uRead = m_psession->recv_with_ts(mem, (i32) mem.get_size(), m_uiTimeStamp, &m_iHaveMore);

            m_iError = (i32) uRead;

			   if(m_iError > 0)
               m_bStreamReceived = true;
			   /* this is to avoid to write to disk some silence before the first RTP packet is returned*/
			   if((m_bStreamReceived) && (m_iError > 0))
            {
               m_memfile.write(mem, uRead);
               m_iError = 0;
            }
            if(tickStart.elapsed() > m_millisTimeout)
            {
               // error = timeout
               return 0; // Eof;
            }
			}
            m_uiTimeStamp += iMp3PacketSize;
         if(tickStart.elapsed() > m_millisTimeout)
         {
            // error = timeout
            return 0; // Eof;
         }
		}
      uRead = min(m_memfile.get_size(), nCount);
      m_memfile.remove_begin(lpBuf, uRead);
      return uRead;

   }

   void file::write(const void * lpBuf, memsize nCount)
   {
      m_psession->send_with_ts(lpBuf, (i32) nCount, m_uiTimeStamp);
      m_uiTimeStamp += (u32) nCount;
   /*   if (clockslide!=0 && user_ts%(160*50)==0){
            ortp_message("Clock sliding of %i miliseconds now",clockslide);
            rtp_session_make_time_distorsion(m_psession->m_psession,clockslide);
         }
         this will simulate a burst of late packets
         if (jitter && (user_ts%(8000)==0)) {
            struct timespec pausetime, remtime;
            ortp_message("Simulating late packets now (%i milliseconds)",jitter);
            pausetime.tv_sec=jitter/1000;
            pausetime.tv_nsec=(jitter%1000)*1000000;
            while(nanosleep(&pausetime,&remtime)==-1 && errno==EINTR){
               pausetime=remtime;
            }*/
      /*      sleep(jitter);
         }*/
   }



   void file::close()
   {
      rtp_session_destroy(m_psession->m_psession);
      ortp_exit();

      ortp_global_stats_display();
   }

   bool file::IsValid() const
   {
      return m_psession->m_psession != nullptr;
   }

#ifdef _DEBUG
void file::assert_ok() const
{
   ::file::file::assert_ok();
}
void file::dump(dump_context & dumpcontext) const
{
   ::file::file::dump(dumpcontext);
}
#endif


void file::set_payload(const char * pszProfile, PayloadType * ptype, i32 iIndex)
{

   m_pprofile = new profile(pszProfile);

   m_psession->set_profile(m_pprofile);

   m_pprofile->set_payload(iIndex, ptype);

   m_psession->set_payload_type(iIndex);

}


} // namespace rtp

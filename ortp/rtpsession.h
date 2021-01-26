 /*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc3550) stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/** 
 * \file rtpsession.h
 * \brief The RtpSession api
 *
 * The RtpSession objects represent a RTP session: once it is configured with 
 * local and remote network addresses and a payload type is given, it let you send
 * and recv a media stream.
**/

#pragma once

typedef enum {
   RTP_SESSION_RECVONLY,
   RTP_SESSION_SENDONLY,
   RTP_SESSION_SENDRECV
} RtpSessionMode;


/*! Jitter buffer parameters
*/
typedef struct _JBParameters{
   i32 min_size; /**< in milliseconds*/
   i32 nom_size; /**< idem */
   i32 max_size; /**< idem */
   bool_t adaptive;
   bool_t pad[3];
   i32 max_packets; /**< max number of packets allowed to be queued in the jitter buffer */
} JBParameters;

typedef struct _JitterControl
{
   i32 count;
   i32 jitt_comp;   /* the user jitt_comp in miliseconds*/
   i32 jitt_comp_ts; /* the jitt_comp converted in rtp time (same unit as timestamp) */
   i32 adapt_jitt_comp_ts;
   i64 slide;
   i64 prev_slide;
   float jitter;
   i32 olddiff;
   float inter_jitter;   /* interarrival jitter as defined in the RFC */
   i32 corrective_step;
   i32 corrective_slide;
   bool_t adaptive;
   bool_t enabled;
} JitterControl;

typedef struct _WaitPoint
{
   ortp_mutex_t lock;
   ortp_cond_t  cond;
   u32 time;
   bool_t wakeup;
} WaitPoint;

typedef struct _RtpTransport
{
   void *data;
   ortp_socket_t (*t_getsocket)(struct _RtpTransport *t);
   i32  (*t_sendto)(struct _RtpTransport *t, mblk_t *msg , i32 flags, const struct sockaddr *to, socklen_t tolen);
   i32  (*t_recvfrom)(struct _RtpTransport *t, mblk_t *msg, i32 flags, struct sockaddr *from, socklen_t *fromlen);
   struct _RtpSession *session; // back pointer to the owning session, set by oRTP
}  RtpTransport;


   
typedef struct _RtpStream
{
   ortp_socket_t socket;
        struct _RtpTransport *tr; 
   i32 sockfamily;
   i32 max_rq_size;
   i32 time_jump;
   u32 ts_jump;
   queue_t rq;
   queue_t tev_rq;
   mblk_t *cached_mp;
   i32 loc_port;
#ifdef ORTP_INET6
   struct sockaddr_storage rem_addr;
#else
   struct sockaddr_in rem_addr;
#endif
   i32 rem_addrlen;
   JitterControl jittctl;
   u32 snd_time_offset;/*the scheduler time when the application send its first timestamp*/   
   u32 snd_ts_offset;   /* the first application timestamp sent by the application */
   u32 snd_rand_offset;   /* a random number added to the user offset to make the stream timestamp*/
   u32 snd_last_ts;   /* the last stream timestamp sended */
   u32 rcv_time_offset; /*the scheduler time when the application ask for its first timestamp*/
   u32 rcv_ts_offset;  /* the first stream timestamp */
   u32 rcv_query_ts_offset;   /* the first user timestamp asked by the application */
   u32 rcv_last_ts;   /* the last stream timestamp got by the application */
   u32 rcv_last_app_ts; /* the last application timestamp asked by the application */   
   u32 rcv_last_ret_ts; /* the timestamp of the last sample returned (only for continuous audio)*/
   u32 hwrcv_extseq; /* last received on socket extended sequence number */
   u32 hwrcv_seq_at_last_SR;
   u32 hwrcv_since_last_SR;
   u32 last_rcv_SR_ts;     /* NTP timestamp (middle 32 bits) of last received SR */
   struct timeval last_rcv_SR_time;   /* time at which last SR was received  */
   u16 snd_seq; /* send sequence number */
   u32 last_rtcp_report_snt_r;   /* the time of the last rtcp report sent, in recv timestamp unit */
   u32 last_rtcp_report_snt_s;   /* the time of the last rtcp report sent, in send timestamp unit */
   u32 rtcp_report_snt_interval; /* the interval in timestamp unit between rtcp report sent */
   u32 last_rtcp_packet_count; /*the sender's octet ::count in the last sent RTCP SR*/
   u32 sent_payload_bytes; /*used for RTCP sender reports*/
   u32 sent_bytes; /* used for bandwidth estimation */
   struct timeval send_bw_start; /* used for bandwidth estimation */
   u32 recv_bytes; /* used for bandwidth estimation */
   struct timeval recv_bw_start; /* used for bandwidth estimation */
   rtp_stats_t stats;
   i32 recv_errno;
   i32 send_errno;
   i32 snd_socket_size;
   i32 rcv_socket_size;
}RtpStream;

typedef struct _RtcpStream
{
   ortp_socket_t socket;
   i32 sockfamily;
        struct _RtpTransport *tr; 

   mblk_t *cached_mp;
#ifdef ORTP_INET6
   struct sockaddr_storage rem_addr;
#else
   struct sockaddr_in rem_addr;
#endif
   i32 rem_addrlen;
   bool_t enabled; /*tells whether we can send RTCP packets */
} RtcpStream;

typedef struct _RtpSession RtpSession;


/**
 * An object representing a bi-directional RTP session.
 * It holds sockets, jitter buffer, various counters (timestamp, sequence numbers...)
 * Applications SHOULD NOT try to read things within the RtpSession object but use
 * instead its public API (the rtp_session_* methods) where RtpSession is used as a 
 * pointer.
 * rtp_session_new() allocates and initialize a RtpSession.
**/
struct _RtpSession
{
   RtpSession *next;   /* next RtpSession, when the session are enqueued by the scheduler */
   i32 mask_pos;   /* the position in the scheduler mask of RtpSession : do not move this field: it is part of the ABI since the session_set macros use it*/
        struct {
     RtpProfile *profile;
     i32 payloadtype;
     u32 ssrc;
     WaitPoint wp;
     i32 telephone_events_pt;   /* the payload type used for telephony events */
   } snd,rcv;
   u32 inc_ssrc_candidate;
   i32 inc_same_ssrc_count;
   i32 hw_recv_pt; /* recv payload type before jitter buffer */
   i32 recv_buf_size;
   RtpSignalTable on_ssrc_changed;
   RtpSignalTable on_payload_type_changed;
   RtpSignalTable on_telephone_event_packet;
   RtpSignalTable on_telephone_event;
   RtpSignalTable on_timestamp_jump;
   RtpSignalTable on_network_error;
   RtpSignalTable on_rtcp_bye;
   struct _OList *signal_tables;
   struct _OList *eventqs;
   msgb_allocator_t allocator;
   RtpStream rtp;
   RtcpStream rtcp;
   RtpSessionMode mode;
   struct _RtpScheduler *sched;
   u32 flags;
   i32 dscp;
   i32 multicast_ttl;
   i32 multicast_loopback;
   void * user_data;
   /* FIXME: Should be a table for all session participants. */
   struct timeval last_recv_time; /* Time of receiving the RTP/RTCP packet. */
   mblk_t *pending;
   /* telephony events extension */
   mblk_t *current_tev;      /* the pending telephony events */
   mblk_t *sd;
   queue_t contributing_sources;
   bool_t symmetric_rtp;
   bool_t permissive; /*use the permissive algorithm*/
   bool_t use_connect; /* use connect() on the socket */
   bool_t ssrc_set;
};
   




/* public API */
RtpSession *rtp_session_new(i32 mode);
void rtp_session_set_scheduling_mode(RtpSession *session, i32 yesno);
void rtp_session_set_blocking_mode(RtpSession *session, i32 yesno);
void rtp_session_set_profile(RtpSession *session, RtpProfile *profile);
void rtp_session_set_send_profile(RtpSession *session,RtpProfile *profile);
void rtp_session_set_recv_profile(RtpSession *session,RtpProfile *profile);
RtpProfile *rtp_session_get_profile(RtpSession *session);
RtpProfile *rtp_session_get_send_profile(RtpSession *session);
RtpProfile *rtp_session_get_recv_profile(RtpSession *session);
i32 rtp_session_signal_connect(RtpSession *session,const char *signal_name, RtpCallback cb, uptr user_data);
i32 rtp_session_signal_disconnect_by_callback(RtpSession *session,const char *signal_name, RtpCallback cb);
void rtp_session_set_ssrc(RtpSession *session, u32 ssrc);
void rtp_session_set_seq_number(RtpSession *session, u16 seq);
u16 rtp_session_get_seq_number(RtpSession *session);

void rtp_session_enable_jitter_buffer(RtpSession *session , bool_t enabled);
bool_t rtp_session_jitter_buffer_enabled(const RtpSession *session);
void rtp_session_set_jitter_buffer_params(RtpSession *session, const JBParameters *par);
void rtp_session_get_jitter_buffer_params(RtpSession *session, JBParameters *par);

/*deprecated jitter control functions*/
void rtp_session_set_jitter_compensation(RtpSession *session, i32 milisec);
void rtp_session_enable_adaptive_jitter_compensation(RtpSession *session, bool_t val);
bool_t rtp_session_adaptive_jitter_compensation_enabled(RtpSession *session);

void rtp_session_set_time_jump_limit(RtpSession *session, i32 miliseconds);
i32 rtp_session_set_local_addr(RtpSession *session,const char *addr, i32 port);
i32 rtp_session_get_local_port(const RtpSession *session);

i32
rtp_session_set_remote_addr_full (RtpSession * session, const char * addr, i32 rtp_port, i32 rtcp_port);
/*same as previous function, old name:*/
i32 rtp_session_set_remote_addr_and_port (RtpSession * session, const char * addr, i32 rtp_port, i32 rtcp_port);
i32 rtp_session_set_remote_addr(RtpSession *session,const char *addr, i32 port);
/* alternatively to the set_remote_addr() and set_local_addr(), an application can give
a valid socket (potentially connect()ed )to be used by the RtpSession */
void rtp_session_set_sockets(RtpSession *session, i32 rtpfd, i32 rtcpfd);
void rtp_session_set_transports(RtpSession *session, RtpTransport *rtptr, RtpTransport *rtcptr);

/*those methods are provided for people who wants to send non-RTP messages using the RTP/RTCP sockets */
ortp_socket_t rtp_session_get_rtp_socket(const RtpSession *session);
ortp_socket_t rtp_session_get_rtcp_socket(const RtpSession *session);


/* QOS / DSCP */
i32 rtp_session_set_dscp(RtpSession *session, i32 dscp);
i32 rtp_session_get_dscp(const RtpSession *session);


/* Multicast methods */
i32 rtp_session_set_multicast_ttl(RtpSession *session, i32 ttl);
i32 rtp_session_get_multicast_ttl(RtpSession *session);

i32 rtp_session_set_multicast_loopback(RtpSession *session, i32 yesno);
i32 rtp_session_get_multicast_loopback(RtpSession *session);



i32 rtp_session_set_send_payload_type(RtpSession *session, i32 paytype);
i32 rtp_session_get_send_payload_type(const RtpSession *session);

i32 rtp_session_get_recv_payload_type(const RtpSession *session);
i32 rtp_session_set_recv_payload_type(RtpSession *session, i32 pt);

i32 rtp_session_set_payload_type(RtpSession *session, i32 pt);

void rtp_session_set_symmetric_rtp (RtpSession * session, bool_t yesno);

void rtp_session_set_connected_mode(RtpSession *session, bool_t yesno);

void rtp_session_enable_rtcp(RtpSession *session, bool_t yesno);

/*low level recv and send functions */
mblk_t * rtp_session_recvm_with_ts (RtpSession * session, u32 user_ts);
mblk_t * rtp_session_create_packet(RtpSession *session,i32 header_size, const u8 * payload, i32 payload_size);
mblk_t * rtp_session_create_packet_with_data(RtpSession *session, u8 * payload, i32 payload_size, void (*freefn)(void*));
mblk_t * rtp_session_create_packet_in_place(RtpSession *session,u8 *buffer, i32 size, void (*freefn)(void*) );
i32 rtp_session_sendm_with_ts (RtpSession * session, mblk_t *mp, u32 userts);
/* high level recv and send functions */
i32 rtp_session_recv_with_ts(RtpSession *session, u8 *buffer, i32 len, u32 ts, i32 *have_more);
i32 rtp_session_send_with_ts(RtpSession *session, const u8 *buffer, i32 len, u32 userts);

/* event API*/
void rtp_session_register_event_queue(RtpSession *session, OrtpEvQueue *q);
void rtp_session_unregister_event_queue(RtpSession *session, OrtpEvQueue *q);


/* IP bandwidth usage estimation functions, returning bits/s*/
float rtp_session_compute_send_bandwidth(RtpSession *session);
float rtp_session_compute_recv_bandwidth(RtpSession *session);

void rtp_session_send_rtcp_APP(RtpSession *session, u8 subtype, const char *name, const u8 *data, i32 datalen);

u32 rtp_session_get_current_send_ts(RtpSession *session);
u32 rtp_session_get_current_recv_ts(RtpSession *session);
void rtp_session_flush_sockets(RtpSession *session);
void rtp_session_release_sockets(RtpSession *session);
void rtp_session_resync(RtpSession *session);
void rtp_session_reset(RtpSession *session);
void rtp_session_destroy(RtpSession *session);

const rtp_stats_t * rtp_session_get_stats(const RtpSession *session);
void rtp_session_reset_stats(RtpSession *session);

void rtp_session_set_data(RtpSession *session, void *data);
void *rtp_session_get_data(const RtpSession *session);

void rtp_session_set_recv_buf_size(RtpSession *session, i32 bufsize);
void rtp_session_set_rtp_socket_send_buffer_size(RtpSession * session, u32 size);
void rtp_session_set_rtp_socket_recv_buffer_size(RtpSession * session, u32 size);

/* in use with the scheduler to convert a timestamp in scheduler time unit (ms) */
u32 rtp_session_ts_to_time(RtpSession *session,u32 timestamp);
u32 rtp_session_time_to_ts(RtpSession *session, i32 millisecs);
/* this function aims at simulating senders with "imprecise" clocks, resulting in 
rtp packets sent with timestamp uncorrelated with the system clock .
This is only availlable to sessions working with the oRTP scheduler */
void rtp_session_make_time_distorsion(RtpSession *session, i32 milisec);

/*RTCP functions */
void rtp_session_set_source_description(RtpSession *session, const char *cname,
   const char *name, const char *email, const char *phone, 
    const char *loc, const char *tool, const char *note);
void rtp_session_add_contributing_source(RtpSession *session, u32 csrc, 
    const char *cname, const char *name, const char *email, const char *phone, 
    const char *loc, const char *tool, const char *note);
void rtp_session_remove_contributing_sources(RtpSession *session, u32 csrc);
mblk_t* rtp_session_create_rtcp_sdes_packet(RtpSession *session);

void rtp_session_get_last_recv_time(RtpSession *session, struct timeval *tv);
i32 rtp_session_bye(RtpSession *session, const char *reason);

i32 rtp_session_get_last_send_error_code(RtpSession *session);
void rtp_session_clear_send_error_code(RtpSession *session);
i32 rtp_session_get_last_recv_error_code(RtpSession *session);
void rtp_session_clear_recv_error_code(RtpSession *session);

/*private */
void rtp_session_init(RtpSession *session, i32 mode);
#define rtp_session_set_flag(session,flag) (session)->flags|=(flag)
#define rtp_session_unset_flag(session,flag) (session)->flags&=~(flag)
void rtp_session_uninit(RtpSession *session);


namespace rtp
{
   class CLASS_DECL_APP_CORE_AUDIO session
   {
   public:
      RtpSession * m_psession;

      session(i32 mode);
   
      void set_scheduling_mode(i32 yesno = 1);
      void set_blocking_mode(i32 yesno = 1);
      void set_profile(profile * pprofile);
      void set_send_profile(profile * pprofile);
      void set_recv_profile(profile * pprofile);


      i32 set_payload_type(i32 pt);

      i32 recv_with_ts(void * buffer, i32 len, u32 ts, i32 *have_more);
      i32 send_with_ts(const void * buffer, i32 len, u32 userts);


      void init(i32 mode);
      void set_flag(i32 flag);
      void unset_flag(i32 flag);
      void uninit();
   };

} // namespace rtp

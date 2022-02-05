#pragma once


namespace rtp
{

   class CLASS_DECL_APP_CORE_AUDIO file :
      public ::file::file
   {
   public:
      ::property_set    m_set;
      profile *            m_pprofile;
      ::rtp::session *            m_psession;
      index                m_iError;
      u32             m_uiTimeStamp;
      u32                m_dwSynchSource;
      millis m_millisTimeout;
      string               m_strListenAddress;
      string               m_strRemoteAddress;
      i32                  m_iListenPort;
      i32                  m_iRemotePort;

      i32                  m_iHaveMore;
      i32                  format;
      i32                  jittcomp;
      bool_t               adapt;
      i32                  clockslide;
      i32                  jitter;
      bool                 m_bStreamReceived;
      ::memory_file   m_memfile;

      file();
      virtual ~file();

      bool rx_open(const char * pszAddress, i32 iPort);
      bool tx_open(const char * pszAddress, i32 iPort);

      virtual bool IsValid() const;

      using ::file::file::read;
      virtual memsize read(void *lpBuf, memsize nCount);

      using ::file::file::write;
      virtual void write(const void * lpBuf, memsize nCount);

      virtual void close();


   #ifdef _DEBUG
      virtual void assert_ok() const;
      virtual void dump(dump_context & dumpcontext) const;
   #endif


      void set_payload(const char * pszProfile, PayloadType * ptype, i32 iIndex = 0);
   };


} // namespace rtp



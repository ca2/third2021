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

#include "framework.h"

#ifdef HAVE_SRTP

#undef PACKAGE_NAME 
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME 
#undef PACKAGE_VERSION

#include "srtp.h"

//#define SRTP_PAD_BYTES 64 /*?? */
//
//static i32  srtp_sendto(RtpTransport *t, mblk_t *m, i32 flags, const struct sockaddr *to, socklen_t tolen){
//   srtp_t srtp=(srtp_t)t->data;
//   i32 slen;
//   err_status_t err;
//   /* enlarge the buffer for srtp to write its data */
//   msgpullup(m,msgdsize(m)+SRTP_PAD_BYTES);
//   slen= (i32) (m->b_wptr-m->b_rptr);
//   err=srtp_protect(srtp,m->b_rptr,&slen);
//   if (err==err_status_ok){
//      return (i32) sendto(t->session->rtp.socket,(const char *)m->b_rptr,slen,flags,to,tolen);
//   }
//   ortp_error("srtp_protect() failed");
//   return -1;
//}
//
//static i32 srtp_recvfrom(RtpTransport *t, mblk_t *m, i32 flags, struct sockaddr *from, socklen_t *fromlen){
//   srtp_t srtp=(srtp_t)t->data;
//   i32 err;
//   i32 slen;
//   err = (i32) recvfrom(t->session->rtp.socket,(char *)m->b_wptr,(i32) (m->b_datap->db_lim-m->b_datap->db_base),flags,from,fromlen);
//   if (err>0){
//
//      /* keep NON-RTP data unencrypted */
//      rtp_header_t *rtp;
//      if (err>=RTP_FIXED_HEADER_SIZE)
//      {
//         rtp = (rtp_header_t*)m->b_wptr;
//         if (rtp->version!=2)
//         {
//            return err;
//         }
//      }
//
//      slen=err;
//      if (srtp_unprotect(srtp,m->b_wptr,&slen)==err_status_ok)
//         return slen;
//      else {
//         ortp_error("srtp_unprotect() failed");
//         return -1;
//      }
//   }
//   return err;
//}
//
//static i32  srtcp_sendto(RtpTransport *t, mblk_t *m, i32 flags, const struct sockaddr *to, socklen_t tolen){
//   srtp_t srtp=(srtp_t)t->data;
//   i32 slen;
//   /* enlarge the buffer for srtp to write its data */
//   msgpullup(m,msgdsize(m)+SRTP_PAD_BYTES);
//   slen=(i32) (m->b_wptr-m->b_rptr);
//   if (srtp_protect_rtcp(srtp,m->b_rptr,&slen)==err_status_ok){
//      return (i32) sendto(t->session->rtcp.socket,(const char *)m->b_rptr,slen,flags,to,tolen);
//   }
//   ortp_error("srtp_protect_rtcp() failed");
//   return -1;
//}
//
//static i32 srtcp_recvfrom(RtpTransport *t, mblk_t *m, i32 flags, struct sockaddr *from, socklen_t *fromlen){
//   srtp_t srtp=(srtp_t)t->data;
//   i32 err;
//   i32 slen;
//   err = (i32) recvfrom(t->session->rtcp.socket,(char *)m->b_wptr,(i32) (m->b_datap->db_lim-m->b_datap->db_base),flags,from,fromlen);
//   if (err>0){
//      slen=err;
//      if (srtp_unprotect_rtcp(srtp,m->b_wptr,&slen)==err_status_ok)
//         return slen;
//      else {
//         ortp_error("srtp_unprotect_rtcp() failed");
//         return -1;
//      }
//   }
//   return err;
//}
//
//ortp_socket_t 
//srtp_getsocket(RtpTransport *t)
//{
//  return t->session->rtp.socket;
//}
//
//ortp_socket_t 
//srtcp_getsocket(RtpTransport *t)
//{
//  return t->session->rtcp.socket;
//}
//
///**
// * Creates a pair of Secure-RTP/Secure-RTCP RtpTransport's.
// * oRTP relies on libsrtp (see http://srtp.sf.net ) for secure RTP encryption.
// * This function creates a RtpTransport object to be used to the RtpSession using
// * rtp_session_set_transport().
// * @srtp: the srtp_t session to be used
// * 
//**/
//i32 srtp_transport_new(srtp_t srtp, RtpTransport **rtpt, RtpTransport **rtcpt ){
//   if (rtpt) {
//      (*rtpt)=(RtpTransport *)ortp_new(RtpTransport,1);
//      (*rtpt)->data=srtp;
//      (*rtpt)->t_getsocket=srtp_getsocket;
//      (*rtpt)->t_sendto=srtp_sendto;
//      (*rtpt)->t_recvfrom=srtp_recvfrom;
//   }
//   if (rtcpt) {
//      (*rtcpt)=(RtpTransport *)ortp_new(RtpTransport,1);
//      (*rtcpt)->data=srtp;
//      (*rtcpt)->t_getsocket=srtcp_getsocket;
//      (*rtcpt)->t_sendto=srtcp_sendto;
//      (*rtcpt)->t_recvfrom=srtcp_recvfrom;
//   }
//   return 0;
//}
//
//err_status_t ortp_srtp_init()
//{
//   return srtp_init();
//}
//
//err_status_t ortp_srtp_create(srtp_t *session, const srtp_policy_t *policy)
//{
//   i32 i;
//   i = srtp_create(session, policy);
//   return (err_status_t) i;
//}
//
//err_status_t ortp_srtp_dealloc(srtp_t session)
//{
//   return srtp_dealloc(session);
//}
//
//err_status_t ortp_srtp_add_stream(srtp_t session, const srtp_policy_t *policy)
//{
//   return srtp_add_stream(session, policy);
//}
//
//bool_t ortp_srtp_supported(){
//   return TRUE;
//}
//
//#else
//
//i32 srtp_transport_new(void *i, RtpTransport **rtpt, RtpTransport **rtcpt ){
//   ortp_error("srtp_transport_new: oRTP has not been compiled with SRTP support.");
//   return -1;
//}
//
//bool_t ortp_srtp_supported(){
//   return FALSE;
//}
//
//i32 ortp_srtp_create(void *i, const void *policy)
//{
//   return -1;
//}
//
//i32 ortp_srtp_dealloc(void *session)
//{
//   return -1;
//}
//
//i32 ortp_srtp_add_stream(void *session, const void *policy)
//{
//   return -1;
//}
//
#endif
//

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
#include "payloadtype.h"
#include "ortp_utils.h"
#include <stdio.h>

char *payload_type_get_rtpmap(PayloadType *payloadtype)
{
   i32 len=(i32)strlen(payloadtype->mime_type)+15;
   char *rtpmap=(char *) ortp_malloc(len);
   if (payloadtype->channels>0)
      snprintf(rtpmap,len,"%s/%i/%i",payloadtype->mime_type,payloadtype->clock_rate,payloadtype->channels);
   else
       snprintf(rtpmap,len,"%s/%i",payloadtype->mime_type,payloadtype->clock_rate);
   return rtpmap;
}

PayloadType *payload_type_new()
{
   PayloadType *newpayload=(PayloadType *)ortp_new0(PayloadType,1);
   newpayload->flags|=PAYLOAD_TYPE_ALLOCATED;
   return newpayload;
}


PayloadType *payload_type_clone(PayloadType * payload)
{
   PayloadType *newpayload=(PayloadType *)ortp_new0(PayloadType,1);
   ::memcpy_dup(newpayload, payload,sizeof(PayloadType));
   newpayload->mime_type=ortp_strdup(payload->mime_type);
   if (payload->recv_fmtp!=nullptr) {
      newpayload->recv_fmtp=ortp_strdup(payload->recv_fmtp);
   }
   if (payload->send_fmtp!=nullptr){
      newpayload->send_fmtp=ortp_strdup(payload->send_fmtp);
   }
   newpayload->flags|=PAYLOAD_TYPE_ALLOCATED;
   return newpayload;
}

static bool_t canWrite(PayloadType *payloadtype){
   if (!(payloadtype->flags & PAYLOAD_TYPE_ALLOCATED)) {
      ortp_error("Cannot change parameters of statically defined payload types: make your"
         " own copy using payload_type_clone() first.");
      return FALSE;
   }
   return TRUE;
}

/**
 * Sets a recv parameters (fmtp) for the PayloadType.
 * This method is provided for applications using RTP with SDP, but
 * actually the ftmp information is not used for RTP processing.
**/
void payload_type_set_recv_fmtp(PayloadType *payloadtype, const char *fmtp){
   if (canWrite(payloadtype)){
      if (payloadtype->recv_fmtp!=nullptr) ortp_free(payloadtype->recv_fmtp);
      if (fmtp!=nullptr) payloadtype->recv_fmtp=ortp_strdup(fmtp);
      else payloadtype->recv_fmtp=nullptr;
   }
}

/**
 * Sets a send parameters (fmtp) for the PayloadType.
 * This method is provided for applications using RTP with SDP, but
 * actually the ftmp information is not used for RTP processing.
**/
void payload_type_set_send_fmtp(PayloadType *payloadtype, const char *fmtp){
   if (canWrite(payloadtype)){
      if (payloadtype->send_fmtp!=nullptr) ortp_free(payloadtype->send_fmtp);
      if (fmtp!=nullptr) payloadtype->send_fmtp=ortp_strdup(fmtp);
      else payloadtype->send_fmtp=nullptr;
   }
}



void payload_type_append_recv_fmtp(PayloadType *payloadtype, const char *fmtp){
   if (canWrite(payloadtype)){
      if (payloadtype->recv_fmtp==nullptr)
         payloadtype->recv_fmtp=ortp_strdup(fmtp);
      else{
         char *tmp=ortp_strdup_printf("%s;%s",payloadtype->recv_fmtp,fmtp);
         ortp_free(payloadtype->recv_fmtp);
         payloadtype->recv_fmtp=tmp;
      }
   }
}

void payload_type_append_send_fmtp(PayloadType *payloadtype, const char *fmtp){
   if (canWrite(payloadtype)){
      if (payloadtype->send_fmtp==nullptr)
         payloadtype->send_fmtp=ortp_strdup(fmtp);
      else{
         char *tmp=ortp_strdup_printf("%s;%s",payloadtype->send_fmtp,fmtp);
         ortp_free(payloadtype->send_fmtp);
         payloadtype->send_fmtp=tmp;
      }
   }
}


/**
 * Frees a PayloadType.
**/
void payload_type_destroy(PayloadType *payloadtype)
{
   if (payloadtype->mime_type) ortp_free((void *) payloadtype->mime_type);
   if (payloadtype->recv_fmtp) ortp_free(payloadtype->recv_fmtp);
   if (payloadtype->send_fmtp) ortp_free(payloadtype->send_fmtp);
   ortp_free(payloadtype);
}

/**
 * Parses a fmtp string such as "profile=0;level=10", finds the value matching
 * parameter param_name, and writes it into result.
 * Despite fmtp strings are not used anywhere within oRTP, this function can
 * be useful for people using RTP streams described from SDP.
 * @param fmtp the fmtp line (format parameters)
 * @param param_name the parameter to search for
 * @param result the value given for the parameter (if found)
 * @param result_len the size allocated to hold the result string
 * @return TRUE if the parameter was found, else FALSE.
**/
bool_t fmtp_get_value(const char *fmtp, const char *param_name, char *result, size_t result_len){
   const char *pos=strstr(fmtp,param_name);
   if (pos){
      const char *equal=strchr(pos,'=');
      if (equal){
         i32 copied;
         const char *end=strchr(equal+1,';');
         if (end==nullptr) end=fmtp+strlen(fmtp); /*assuming this is the last param */
         size_t len = end-(equal+1);
         copied=(i32) min(result_len-1, len);
         strncpy(result,equal+1,copied);
         result[copied]='\0';
         return TRUE;
      }
   }
   return FALSE;
}


i32 rtp_profile_get_payload_number_from_mime(RtpProfile *profile,const char *mime)
{
   PayloadType *payloadtype;
   i32 i;
   for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++)
   {
      payloadtype=rtp_profile_get_payload(profile,i);
      if (payloadtype!=nullptr)
      {
         if (strcasecmp(payloadtype->mime_type,mime)==0){
            return i;
         }
      }
   }
   return -1;
}

i32 rtp_profile_find_payload_number(RtpProfile*profile,const char *mime,i32 rate,i32 channels)
{
   i32 i;
   PayloadType *payloadtype;
   for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++)
   {
      payloadtype=rtp_profile_get_payload(profile,i);
      if (payloadtype!=nullptr)
      {
         if (strcasecmp(payloadtype->mime_type,mime)==0 &&
             payloadtype->clock_rate==rate &&
             (payloadtype->channels==channels || channels<=0 || payloadtype->channels<=0)) {
            /*we don't look at channels if it is undefined
            ie a negative or zero value*/

            return i;
         }
      }
   }
   return -1;
}

                              
i32 rtp_profile_get_payload_number_from_rtpmap(RtpProfile * profile, const char * rtpmap)
{
   
   i32 iClockRate;
   i32 iChannels;
   i32 iRet;
   char* subtype = ortp_strdup( rtpmap );
   char* rate_str = nullptr;
   char* chan_str = nullptr;


   /* find the slash after the subtype */
   rate_str = strchr(subtype, '/');
   if (rate_str && strlen(rate_str)>1) {
      *rate_str = 0;
      rate_str++;

      /* Look for another slash */
      chan_str = strchr(rate_str, '/');
      if (chan_str && strlen(chan_str)>1) {
         *chan_str = 0;
         chan_str++;
      } else {
         chan_str = nullptr;
      }
   } else {
      rate_str = nullptr;
   }

   // Use default clock rate if none given
   if (rate_str) iClockRate = atoi(rate_str);
   else iClockRate = 8000;

   // Use default number of channels if none given
   if (chan_str) iChannels = atoi(chan_str);
   else iChannels = -1;

   //debug_print("Searching for payload %s at freq %i with %i channels\n",subtype,clock_rate,ch1annels);
   iRet=rtp_profile_find_payload_number(profile,subtype,iClockRate,iChannels);
   ortp_free(subtype);
   return iRet;
}

PayloadType * rtp_profile_find_payload(RtpProfile *prof,const char *mime,i32 rate,i32 channels)
{
   i32 i;
   i=rtp_profile_find_payload_number(prof,mime,rate,channels);
   if (i>=0) return rtp_profile_get_payload(prof,i);
   return nullptr;
}


PayloadType * rtp_profile_get_payload_from_mime(RtpProfile *profile,const char *mime)
{
   i32 payloadtype;
   payloadtype=rtp_profile_get_payload_number_from_mime(profile,mime);
   if (payloadtype==-1) return nullptr;
   else return rtp_profile_get_payload(profile, payloadtype);
}


PayloadType * rtp_profile_get_payload_from_rtpmap(RtpProfile *profile, const char *rtpmap)
{
   i32 payloadtype = rtp_profile_get_payload_number_from_rtpmap(profile,rtpmap);
   if (payloadtype==-1) return nullptr;
   else return rtp_profile_get_payload(profile, payloadtype);
}

i32 rtp_profile_move_payload(RtpProfile *prof,i32 oldpos,i32 newpos){
   prof->payload[newpos]=prof->payload[oldpos];
   prof->payload[oldpos]=nullptr;
   return 0;
}

RtpProfile * rtp_profile_new(const char *name)
{
   RtpProfile *prof=(RtpProfile*)ortp_new0(RtpProfile,1);
   rtp_profile_set_name(prof,name);
   return prof;
}

/**
 *   Assign payload type number index to payload type desribed in payloadtype for the RTP profile profile.
 * @param profile a RTP profile
 * @param idx the payload type number
 * @param payloadtype the payload type description
 *
**/
void rtp_profile_set_payload(RtpProfile *profile, i32 idx, PayloadType *payloadtype){
   if (idx<0 || idx>=RTP_PROFILE_MAX_PAYLOADS) {
      ortp_error("Bad index %i",idx);
      return;
   }
   profile->payload[idx]=payloadtype;
}

/**
 * Initialize the profile to the is_empty profile (all payload type are unassigned).
 *@param profile a RTP profile
 *
**/
void rtp_profile_clear_all(RtpProfile *profile){
   i32 i;
   for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++){
      profile->payload[i]=0;
   }
}


/**
 * set a name to the rtp profile. (This is not required)
 * @param profile a rtp profile object
 * @param name a string
 *
**/
void rtp_profile_set_name(RtpProfile *profile, const char *name){
   if (profile->name!=nullptr) ortp_free((void *) profile->name);
   profile->name=ortp_strdup(name);
}

/* ! payload are not cloned*/
RtpProfile * rtp_profile_clone(RtpProfile *prof)
{
   i32 i;
   PayloadType *payloadtype;
   RtpProfile *newprof=rtp_profile_new(prof->name);
   for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++){
      payloadtype=rtp_profile_get_payload(prof,i);
      if (payloadtype!=nullptr){
         rtp_profile_set_payload(newprof,i, payloadtype);
      }
   }
   return newprof;
}


/*clone a profile and its payloads */
RtpProfile * rtp_profile_clone_full(RtpProfile *prof)
{
   i32 i;
   PayloadType *payloadtype;
   RtpProfile *newprof=rtp_profile_new(prof->name);
   for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++){
      payloadtype=rtp_profile_get_payload(prof,i);
      if (payloadtype!=nullptr){
         rtp_profile_set_payload(newprof,i,payload_type_clone(payloadtype));
      }
   }
   return newprof;
}

void rtp_profile_destroy(RtpProfile *prof)
{
   i32 i;
   PayloadType *payload;
   if (prof->name) {
      ortp_free((void *) prof->name);
      prof->name = nullptr;
   }
   for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++)
   {
      payload=rtp_profile_get_payload(prof,i);
      if (payload!=nullptr && (payload->flags & PAYLOAD_TYPE_ALLOCATED))
         payload_type_destroy(payload);
   }
   ortp_free(prof);
}


namespace rtp
{
   profile::profile(const char * pszName)
   {
      m_pprofile = rtp_profile_new(pszName);
   }

   void profile::set_payload(i32 iIndex, PayloadType * ptype)
   {
      rtp_profile_set_payload(m_pprofile, iIndex, ptype);
   }

} // namespace rtp

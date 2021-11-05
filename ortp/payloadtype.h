﻿/*
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
 * \file "payloadtype.h".h
 * \brief Using and creating standart and custom RTP profiles
 *
**/

#pragma once

/* flags for PayloadType::flags */

#define   PAYLOAD_TYPE_ALLOCATED (1)
   /* private flags for future use by ortp */
#define   PAYLOAD_TYPE_PRIV1 (1<<1)
#define   PAYLOAD_TYPE_PRIV2 (1<<2)
#define   PAYLOAD_TYPE_PRIV3 (1<<3)
   /* user flags, can be used by the application on top of oRTP */
#define   PAYLOAD_TYPE_USER_FLAG_0 (1<<4)
#define   PAYLOAD_TYPE_USER_FLAG_1 (1<<5)
#define   PAYLOAD_TYPE_USER_FLAG_2 (1<<6)
   /* ask for more if you need*/

#define PAYLOAD_AUDIO_CONTINUOUS 0
#define PAYLOAD_AUDIO_PACKETIZED 1
#define PAYLOAD_VIDEO 2
#define PAYLOAD_OTHER 3  /* ?? */

struct _PayloadType
{
   i32 type; /**< one of PAYLOAD_* macros*/
   i32 clock_rate; /**< rtp clock rate*/
   char bits_per_sample;   /* in case of continuous audio data */
   char *zero_pattern;
   i32 pattern_length;
   /* other useful information for the application*/
   i32 normal_bitrate;   /*in bit/s */
   const char *mime_type; /**<actually the submime, ex: pcm, pcma, gsm*/
   i32 channels; /**< number of channels of audio */
   char *recv_fmtp; /* various format parameters for the incoming stream */
   char *send_fmtp; /* various format parameters for the outgoing stream */
   i32 flags;
   void *user_data;
};

#ifndef PayloadType_defined
#define PayloadType_defined
typedef struct _PayloadType PayloadType;
#endif

#define payload_type_set_flag(pt,flag) (point)->flags|=((i32)flag)
#define payload_type_unset_flag(pt,flag) (point)->flags&=(~(i32)flag)
#define payload_type_get_flags(point)   (point)->flags

#define RTP_PROFILE_MAX_PAYLOADS 128

/**
 * The RTP profile is a table RTP_PROFILE_MAX_PAYLOADS entries to make the matching
 * between RTP payload type number and the PayloadType that defines the type of
 * media.
**/
struct _RtpProfile
{
   const char *name;
   PayloadType *payload[RTP_PROFILE_MAX_PAYLOADS];
};


typedef struct _RtpProfile RtpProfile;

PayloadType *payload_type_new();
PayloadType *payload_type_clone(PayloadType * payload);
char *payload_type_get_rtpmap(PayloadType *pt);
void payload_type_destroy(PayloadType *pt);
void payload_type_set_recv_fmtp(PayloadType *pt, const char *fmtp);
void payload_type_set_send_fmtp(PayloadType *pt, const char *fmtp);
void payload_type_append_recv_fmtp(PayloadType *pt, const char *fmtp);
void payload_type_append_send_fmtp(PayloadType *pt, const char *fmtp);


bool_t fmtp_get_value(const char *fmtp, const char *param_name, char *result, size_t result_len);

extern CLASS_DECL_APP_CORE_AUDIO RtpProfile av_profile;

#define payload_type_set_user_data(pt,p)   (point)->user_data=(p)
#define payload_type_get_user_data(point)      ((point)->user_data)

#define rtp_profile_get_name(profile)    (const char*)((profile)->name)

void rtp_profile_set_payload(RtpProfile *prof, i32 idx, PayloadType *pt);

/**
 *   set payload type number @index unassigned in the profile.
 *
 *@param profile an RTP profile
 *@param index   the payload type number
**/
#define rtp_profile_clear_payload(profile,index) \
   rtp_profile_set_payload(profile,index,nullptr)

/* I prefer have this function inlined because it is very often called in the code */
/**
 *
 *   Gets the payload description of the payload type @index in the profile.
 *
 *param profile an RTP profile (a #RtpProfile object)
 *param index   the payload type number
 *return the payload description (a PayloadType object)
**/
static inline PayloadType * rtp_profile_get_payload(RtpProfile *prof, i32 idx){
   if (idx<0 || idx>=RTP_PROFILE_MAX_PAYLOADS) {
      return nullptr;
   }
   return prof->payload[idx];
}
void rtp_profile_clear_all(RtpProfile *prof);
void rtp_profile_set_name(RtpProfile *prof, const char *name);
PayloadType * rtp_profile_get_payload_from_mime(RtpProfile *profile,const char *mime);
PayloadType * rtp_profile_get_payload_from_rtpmap(RtpProfile *profile, const char *rtpmap);
i32 rtp_profile_get_payload_number_from_mime(RtpProfile *profile,const char *mime);
i32 rtp_profile_get_payload_number_from_rtpmap(RtpProfile *profile, const char *rtpmap);
i32 rtp_profile_find_payload_number(RtpProfile *prof,const char *mime,i32 rate, i32 channels);
PayloadType * rtp_profile_find_payload(RtpProfile *prof,const char *mime,i32 rate, i32 channels);
i32 rtp_profile_move_payload(RtpProfile *prof,i32 oldpos,i32 newpos);

RtpProfile * rtp_profile_new(const char *name);
/* clone a profile, payload are not cloned */
RtpProfile * rtp_profile_clone(RtpProfile *prof);


/*clone a profile and its payloads (ie payload type are newly allocated, not reusing payload types of the context_object profile) */
RtpProfile * rtp_profile_clone_full(RtpProfile *prof);
/* frees the profile and all its PayloadTypes*/
void rtp_profile_destroy(RtpProfile *prof);


/* some payload types */
/* audio */
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_pcmu8000;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_pcma8000;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_pcm8000;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_l16_mono;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_l16_stereo;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_lpc1016;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_g729;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_g7231;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_g7221;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_g726_40;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_g726_32;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_g726_24;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_g726_16;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_gsm;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_lpc;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_lpc1015;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_speex_nb;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_speex_wb;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_speex_uwb;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_ilbc;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_amr;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_amrwb;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_truespeech;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_evrc0;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_evrcb0;

extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_mp3_128;

/* video */
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_mpv;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_h261;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_h263;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_h263_1998;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_h263_2000;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_mp4v;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_theora;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_h264;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_x_snow;
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_t140;

/* non standard file transfer over UDP */
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_x_udpftp;

/* telephone-event */
extern CLASS_DECL_APP_CORE_AUDIO PayloadType payload_type_telephone_event;



namespace rtp
{

   class CLASS_DECL_APP_CORE_AUDIO profile
   {
   public:
      RtpProfile * m_pprofile;

      profile(const char * pszName);

      void set_payload(i32 idx, PayloadType *pt);

   };

} // namespace rtp

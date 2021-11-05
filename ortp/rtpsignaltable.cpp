/*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc1889) stack.
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
#include "rtpsession.h"
#include "ortp_utils.h"


void rtp_signal_table_init(RtpSignalTable *table,RtpSession *session, const char *signal_name)
{
   __memset(table,0,sizeof(RtpSignalTable));
   table->session=session;
   table->signal_name=signal_name;
   session->signal_tables=o_list_append(session->signal_tables,(void*)table);
}

i32 rtp_signal_table_add(RtpSignalTable *table,RtpCallback cb, uptr user_data)
{
   i32 i;
   
   for (i=0;i<RTP_CALLBACK_TABLE_MAX_ENTRIES;i++){
      if (table->callback[i]==nullptr){
         table->callback[i]=cb;
         table->user_data[i]=user_data;
         table->count++;
         return 0;
      }
   }
   return -1;
}

void rtp_signal_table_emit(RtpSignalTable *table)
{
   i32 i,ca;
   
   for (i=0,ca=0;ca<table->count;i++){
      if (table->callback[i]!=nullptr){
         ca++;   /*I like it*/
         table->callback[i](table->session,table->user_data[i]);
      }
   }
}

void rtp_signal_table_emit2(RtpSignalTable *table,uptr arg)
{
   i32 i,ca;
   
   for (i=0,ca=0;ca<table->count;i++){
      if (table->callback[i]!=nullptr){
         ca++;   /*I like it*/
         table->callback[i](table->session,arg,table->user_data[i]);
      }
   }
}

void rtp_signal_table_emit3(RtpSignalTable *table, uptr arg1, uptr arg2)
{
   i32 i,ca;
   
   for (i=0,ca=0;ca<table->count;i++){
      if (table->callback[i]!=nullptr){
         ca++;   /*I like it*/
         table->callback[i](table->session,arg1,arg2,table->user_data[i]);
      }
   }
}

i32 rtp_signal_table_remove_by_callback(RtpSignalTable *table,RtpCallback cb)
{
   i32 i;
   
   for (i=0;i<RTP_CALLBACK_TABLE_MAX_ENTRIES;i++){
      if (table->callback[i]==cb){
         table->callback[i]=nullptr;
         table->user_data[i]=0;
         table->count--;
         return 0;
      }
   }
   return -1;
}

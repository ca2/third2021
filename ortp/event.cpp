﻿ /*
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

RtpEndpoint *rtp_endpoint_new(struct sockaddr *addr, socklen_t addrlen){
   RtpEndpoint *endpoint=(RtpEndpoint *) ortp_new(RtpEndpoint,1);
   if (sizeof(endpoint->addr)<addrlen){
      ortp_free(endpoint);
      ortp_fatal("Bad socklen_t size !");
      return nullptr;
   }
   ::memcpy_dup(&endpoint->addr,addr,addrlen);
   endpoint->addrlen=addrlen;
   return endpoint;
}

void rtp_endpt_destroy(RtpEndpoint *endpoint){
   ortp_free(endpoint);
}

RtpEndpoint *rtp_endpoint_dup(const RtpEndpoint *endpoint){
   return rtp_endpoint_new((struct sockaddr*)&endpoint->addr,endpoint->addrlen);
}

OrtpEvent * ortp_event_new(uptr type){
   const i32 size=sizeof(OrtpEventType)+sizeof(OrtpEventData);
   mblk_t *m=allocb(size,0);
   __memset(m->b_wptr,0,size);
   *((OrtpEventType*)m->b_wptr)=type;
   return m;
}

OrtpEvent *ortp_event_dup(OrtpEvent *ev){
#if 0
   OrtpEvent *nev=dupb(ev);
#else
   OrtpEvent *nev = ortp_event_new(ortp_event_get_type(ev));
   OrtpEventData * ed = ortp_event_get_data(ev);
   OrtpEventData * edv = ortp_event_get_data(nev);

   if (ed->endpoint) edv->endpoint = rtp_endpoint_dup(ed->endpoint);
   if (ed->packet) edv->packet = copymsg(ed->packet);
   edv->info.telephone_event = ed->info.telephone_event;
#endif
   return nev;
}

OrtpEventType ortp_event_get_type(const OrtpEvent *ev){
   return ((OrtpEventType*)ev->b_rptr)[0];
}

OrtpEventData * ortp_event_get_data(OrtpEvent *ev){
   return (OrtpEventData*)(ev->b_rptr+sizeof(OrtpEventType));
}

void ortp_event_destroy(OrtpEvent *ev){
   OrtpEventData *d=ortp_event_get_data(ev);
   if (ev->b_datap->db_ref==1){
      if (d->packet)    freemsg(d->packet);
      if (d->endpoint) rtp_endpt_destroy(d->endpoint);
   }
   freemsg(ev);
}

OrtpEvQueue * ortp_ev_queue_new(){
   OrtpEvQueue *q=(OrtpEvQueue *)ortp_new(OrtpEvQueue,1);
   qinit(&q->q);
   ortp_mutex_init(&q->mutex,nullptr);
   return q;
}

void ortp_ev_queue_flush(OrtpEvQueue * qp){
   OrtpEvent *ev;
   while((ev=ortp_ev_queue_get(qp))!=nullptr){
      ortp_event_destroy(ev);
   }
}

OrtpEvent * ortp_ev_queue_get(OrtpEvQueue *q){
   OrtpEvent *ev;
   ortp_single_lock(&q->mutex);
   ev=getq(&q->q);
   ortp_mutex_unlock(&q->mutex);
   return ev;
}

void ortp_ev_queue_destroy(OrtpEvQueue * qp){
   ortp_ev_queue_flush(qp);
   ortp_mutex_destroy(&qp->mutex);
   ortp_free(qp);
}

void ortp_ev_queue_put(OrtpEvQueue *q, OrtpEvent *ev){
   ortp_single_lock(&q->mutex);
   putq(&q->q,ev);
   ortp_mutex_unlock(&q->mutex);
}


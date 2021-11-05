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
#include "rtp.h"
#include "str_utils.h"
#include "ortp_utils.h"
#include <stdio.h>

void qinit(queue_t *q){
   mblk_init(&q->_q_stopper);
   q->_q_stopper.b_next=&q->_q_stopper;
   q->_q_stopper.b_prev=&q->_q_stopper;
   q->q_mcount=0;
}

void mblk_init(mblk_t *mp)
{
   mp->b_cont=mp->b_prev=mp->b_next=nullptr;
   mp->b_rptr=mp->b_wptr=nullptr;
   mp->reserved1=0;
   mp->reserved2=0;
}

dblk_t *datab_alloc(i32 size){
   dblk_t *db;
   i32 total_size=sizeof(dblk_t)+size;
   db=(dblk_t *) ortp_malloc(total_size);
   db->db_base=(u8*)db+sizeof(dblk_t);
   db->db_lim=db->db_base+size;
   db->db_ref=1;
   db->db_freefn=nullptr;   /* the buffer pted by db_base must never be freed !*/
   return db;
}

static inline void datab_ref(dblk_t *d){
   d->db_ref++;
}

static inline void datab_unref(dblk_t *d){
   d->db_ref--;
   if (d->db_ref==0){
      if (d->db_freefn!=nullptr)
         d->db_freefn(d->db_base);
      ortp_free(d);
   }
}


mblk_t *allocb(i32 size, i32 pri)
{
   UNREFERENCED_PARAMETER(pri);
   mblk_t *mp;
   dblk_t *datab;
   
   mp=(mblk_t *) ortp_malloc(sizeof(mblk_t));
   mblk_init(mp);
   datab=datab_alloc(size);
   
   mp->b_datap=datab;
   mp->b_rptr=mp->b_wptr=datab->db_base;
   mp->b_next=mp->b_prev=mp->b_cont=nullptr;
   return mp;
}

mblk_t *esballoc(u8 *buf, i32 size, i32 pri, void (*freefn)(void*) )
{
   UNREFERENCED_PARAMETER(pri);
   mblk_t *mp;
   dblk_t *datab;
   
   mp=(mblk_t *) ortp_malloc(sizeof(mblk_t));
   mblk_init(mp);
   datab=(dblk_t *) ortp_malloc(sizeof(dblk_t));
   

   datab->db_base=buf;
   datab->db_lim=buf+size;
   datab->db_ref=1;
   datab->db_freefn=freefn;
   
   mp->b_datap=datab;
   mp->b_rptr=mp->b_wptr=buf;
   mp->b_next=mp->b_prev=mp->b_cont=nullptr;
   return mp;
}

   
void freeb(mblk_t *mp)
{
   return_if_fail(mp->b_datap!=nullptr);
   return_if_fail(mp->b_datap->db_base!=nullptr);
   
   datab_unref(mp->b_datap);
   ortp_free(mp);
}

void freemsg(mblk_t *mp)
{
   mblk_t *tmp1,*tmp2;
   tmp1=mp;
   while(tmp1!=nullptr)
   {
      tmp2=tmp1->b_cont;
      freeb(tmp1);
      tmp1=tmp2;
   }
}

mblk_t *dupb(mblk_t *mp)
{
   mblk_t *newm;
   return_val_if_fail(mp->b_datap!=nullptr,nullptr);
   return_val_if_fail(mp->b_datap->db_base!=nullptr,nullptr);
   
   datab_ref(mp->b_datap);
   newm=(mblk_t *) ortp_malloc(sizeof(mblk_t));
   mblk_init(newm);
   newm->reserved1=mp->reserved1;
   newm->reserved2=mp->reserved2;
   newm->b_datap=mp->b_datap;
   newm->b_rptr=mp->b_rptr;
   newm->b_wptr=mp->b_wptr;
   return newm;
}

/* duplicates a complex mblk_t */
mblk_t   *dupmsg(mblk_t* m)
{
   mblk_t *newm=nullptr,*mp,*prev;
   prev=newm=dupb(m);
   m=m->b_cont;
   while (m!=nullptr){
      mp=dupb(m);
      prev->b_cont=mp;
      prev=mp;
      m=m->b_cont;
   }
   return newm;
}

void putq(queue_t *q,mblk_t *mp)
{
   q->_q_stopper.b_prev->b_next=mp;
   mp->b_prev=q->_q_stopper.b_prev;
   mp->b_next=&q->_q_stopper;
   q->_q_stopper.b_prev=mp;
   q->q_mcount++;
}

mblk_t *getq(queue_t *q)
{
   mblk_t *tmp;
   tmp=q->_q_stopper.b_next;
   if (tmp==&q->_q_stopper) return nullptr;
   q->_q_stopper.b_next=tmp->b_next;
   tmp->b_next->b_prev=&q->_q_stopper;
   tmp->b_prev=nullptr;
   tmp->b_next=nullptr;
   q->q_mcount--;
   return tmp;
}

mblk_t * peekq(queue_t *q){
   mblk_t *tmp;
   tmp=q->_q_stopper.b_next;
   if (tmp==&q->_q_stopper) return nullptr;
   return tmp;
}

/* insert mp in q just before emp */
void insq(queue_t *q,mblk_t *emp, mblk_t *mp)
{
   if (emp==nullptr){
      putq(q,mp);
      return;
   }
   q->q_mcount++;
   emp->b_prev->b_next=mp;
   mp->b_prev=emp->b_prev;
   emp->b_prev=mp;
   mp->b_next=emp;   
}

void remq(queue_t *q, mblk_t *mp){
   q->q_mcount--;
   mp->b_prev->b_next=mp->b_next;
   mp->b_next->b_prev=mp->b_prev;
   mp->b_next=nullptr;
   mp->b_prev=nullptr;
}

/* remove and free all messages in the q */
void flushq(queue_t *q, i32 how)
{
   UNREFERENCED_PARAMETER(how);
   mblk_t *mp;
   
   while ((mp=getq(q))!=nullptr)
   {
      freemsg(mp);
   }
}

i32 msgdsize(const mblk_t *mp)
{
   i32 msgsize=0;
   while(mp!=nullptr){
      msgsize+=(i32) (mp->b_wptr-mp->b_rptr);
      mp=mp->b_cont;
   }
   return msgsize;
}

void msgpullup(mblk_t *mp,i32 len)
{
   mblk_t *firstm=mp;
   dblk_t *db;
   i32 wlen=0;

   if (mp->b_cont==nullptr && len==-1) return;   /*nothing to do, message is not fragmented */

   if (len==-1) len=msgdsize(mp);
   db=datab_alloc(len);
   while(wlen<len && mp!=nullptr){
      i32 remain=len-wlen;
      i32 mlen=(i32) (mp->b_wptr-mp->b_rptr);
      if (mlen<=remain){
         ::memcpy_dup(&db->db_base[wlen],mp->b_rptr,mlen);
         wlen+=mlen;
         mp=mp->b_cont;
      }else{
         ::memcpy_dup(&db->db_base[wlen],mp->b_rptr,remain);
         wlen+=remain;
      }
   }
   /*set firstm to payloadtype to the new datab */
   freemsg(firstm->b_cont);
   firstm->b_cont=nullptr;
   datab_unref(firstm->b_datap);
   firstm->b_datap=db;
   firstm->b_rptr=db->db_base;
   firstm->b_wptr=firstm->b_rptr+wlen;
}


mblk_t *copyb(mblk_t *mp)
{
   mblk_t *newm;
   i32 len=(i32) (mp->b_wptr-mp->b_rptr);
   newm=allocb(len,BPRI_MED);
   ::memcpy_dup(newm->b_wptr,mp->b_rptr,len);
   newm->b_wptr+=len;
   return newm;
}

mblk_t *copymsg(mblk_t *mp)
{
   mblk_t *newm=0,*m;
   m=newm=copyb(mp);
   mp=mp->b_cont;
   while(mp!=nullptr){
      m->b_cont=copyb(mp);
      m=m->b_cont;
      mp=mp->b_cont;
   }
   return newm;
}

mblk_t * appendb(mblk_t *mp, const char *data, i32 size, bool_t pad){
   i32 padcnt=0;
   i32 i;
   if (pad){
      padcnt= (i32)(4L-( (uptr)(((uptr)mp->b_wptr)+size) % 4L)) % 4L;
   }
   if ((mp->b_wptr + size +padcnt) > mp->b_datap->db_lim){
      /* buffer is not large enough: append a new block (with the same size ?)*/
      i32 plen=(i32)((char*)mp->b_datap->db_lim - (char*) mp->b_datap->db_base);
      mp->b_cont=allocb(max(plen,size),0);
      mp=mp->b_cont;
   }
   if (size) ::memcpy_dup(mp->b_wptr,data,size);
   mp->b_wptr+=size;
   for (i=0;i<padcnt;i++){
      mp->b_wptr[0]=0;
      mp->b_wptr++;
   }
   return mp;
}

void msgappend(mblk_t *mp, const char *data, i32 size, bool_t pad){
   while(mp->b_cont!=nullptr) mp=mp->b_cont;
   appendb(mp,data,size,pad);
}

mblk_t *concatb(mblk_t *mp, mblk_t *newm){
   while (mp->b_cont!=nullptr) mp=mp->b_cont;
   mp->b_cont=newm;
   while(newm->b_cont!=nullptr) newm=newm->b_cont;
   return newm;
}

void msgb_allocator_init(msgb_allocator_t *a){
   qinit(&a->q);
}

mblk_t *msgb_allocator_alloc(msgb_allocator_t *a, i32 size){
   queue_t *q=&a->q;
   mblk_t *m,*found=nullptr;

   /*lookup for an unused msgb (data block with ref count ==1)*/
   for(m=qbegin(q);!qend(q,m);m=qnext(q,m)){
      if (m->b_datap->db_ref==1 && m->b_datap->db_lim-m->b_datap->db_base>=size){
         found=m;
         break;
      }
   }
   if (found==nullptr){
      found=allocb(size,0);
      putq(q,found);
   }
   return dupb(found);
}

void msgb_allocator_uninit(msgb_allocator_t *a){
   flushq(&a->q,-1);
}


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
#include "ortp_utils.h"
#include <stdio.h>

#if   defined(_WIN32) && !defined(_WIN32_WCE)
#include <process.h>
#endif



#ifdef APPLEOS
#define _unlink unlink
#endif

static void *ortp_libc_malloc(size_t sz){
   return malloc(sz);
}

static void *ortp_libc_realloc(void *ptr, size_t sz){
   return realloc(ptr,sz);
}

static void ortp_libc_free(void*ptr){
   free(ptr);
}

static bool_t allocator_used=FALSE;

static OrtpMemoryFunctions ortp_allocator={
   ortp_libc_malloc,
   ortp_libc_realloc,
   ortp_libc_free
};

void ortp_set_memory_functions(OrtpMemoryFunctions *functions){
   if (allocator_used){
      ortp_fatal("ortp_set_memory_functions() must be called before "
      "first use of ortp_malloc or ortp_realloc");
      return;
   }
   ortp_allocator=*functions;
}

void* ortp_malloc(size_t sz){
   allocator_used=TRUE;
   return ortp_allocator.malloc_fun(sz);
}

void* ortp_realloc(void *ptr, size_t sz){
   allocator_used=TRUE;
   return ortp_allocator.realloc_fun(ptr,sz);
}

void ortp_free(void* ptr){
   ortp_allocator.free_fun(ptr);
}

void * ortp_malloc0(size_t size){
   void *ptr=ortp_malloc(size);
   __memset(ptr,0,size);
   return ptr;
}

char * ortp_strdup(const char *tmp){
   size_t sz;
   char *ret;
   if (tmp==nullptr)
     return nullptr;
   sz=strlen(tmp)+1;
   ret=(char*)ortp_malloc(sz);
   strcpy(ret,tmp);
   ret[sz-1]='\0';
   return ret;
}

/*
 * this method is an utility method that calls fnctl() on UNIX or
 * ioctlsocket on Win32.
 * i32 retrun the result of the system method
 */
i32 set_non_blocking_socket (ortp_socket_t sock)
{


#if   !defined(_WIN32) && !defined(_WIN32_WCE)
   return fcntl (sock, F_SETFL, O_NONBLOCK);
#else
   u_long nonBlock = 1;
   return ioctlsocket(sock, FIONBIO , &nonBlock);
#endif
}


/*
 * this method is an utility method that calls close() on UNIX or
 * closesocket on Win32.
 * i32 retrun the result of the system method
 */
i32 close_socket(ortp_socket_t sock){
#if   !defined(_WIN32) && !defined(_WIN32_WCE)
   return close (sock);
#else
   return closesocket(sock);
#endif
}



#if   !defined(_WIN32) && !defined(_WIN32_WCE)
   /* Use UNIX inet_aton method */
#else
   i32 inet_aton (const char * cp, struct in_addr * addr)
   {
      u32 retval;

      retval = inet_addr (cp);

      if (retval == INADDR_NONE)
      {
         return -1;
      }
      else
      {
         addr->S_un.S_addr = retval;
         return 1;
      }
   }
#endif

char *ortp_strndup(const char *str,i32 n){
   i32 min=min((i32)strlen(str),n)+1;
   char *ret=(char*)ortp_malloc(min);
   ansi_count_copy(ret,str,n);
   ret[min-1]='\0';
   return ret;
}

#if   !defined(_WIN32) && !defined(_WIN32_WCE)
i32 __ortp_thread_join(ortp_thread_t thread, void **ptr){
   i32 err=pthread_join(thread,ptr);
   if (err!=0) {
      ortp_error("pthread_join error: %s",strerror(err));
   }
   return err;
}

i32 __ortp_thread_create(pthread_t *thread, pthread_attr_t *attr, void * (*routine)(void*), void *arg){
   pthread_attr_t my_attr;
   pthread_attr_init(&my_attr);
   if (attr)
      my_attr = *attr;
#ifdef ORTP_DEFAULT_THREAD_STACK_SIZE
   if (ORTP_DEFAULT_THREAD_STACK_SIZE!=0)
      pthread_attr_setstacksize(&my_attr, ORTP_DEFAULT_THREAD_STACK_SIZE);
#endif
   return pthread_create(thread, &my_attr, routine, arg);
}

#endif
#if   defined(_WIN32) || defined(_WIN32_WCE)

i32 WIN_mutex_init(ortp_mutex_t *mutex, void *attr)
{
   UNREFERENCED_PARAMETER(attr);
   *mutex=CreateMutex(nullptr, FALSE, nullptr);
   return 0;
}

i32 WIN_single_lock(ortp_mutex_t * hMutex)
{
   WaitForSingleObject(*hMutex, U32_INFINITE_TIMEOUT); /* == WAIT_TIMEOUT; */
   return 0;
}

i32 WIN_mutex_unlock(ortp_mutex_t * hMutex)
{
   ReleaseMutex(*hMutex);
   return 0;
}

i32 WIN_mutex_destroy(ortp_mutex_t * hMutex)
{
   CloseHandle(*hMutex);
   return 0;
}

typedef struct thread_param{
   void * (*func)(void *);
   void * arg;
}thread_param_t;

static u32 WINAPI thread_starter(void *data){
   thread_param_t *params=(thread_param_t*)data;
   void *ret=params->func(params->arg);
   ortp_free(data);
   return (u32) (uptr)ret;
}

#if defined _WIN32_WCE
#    define _beginthreadex   create_thread
#    define   _endthreadex   ExitThread
#endif

i32 WIN_thread_create(ortp_thread_t *th, void *attr, void * (*func)(void *), void *data)
{
   UNREFERENCED_PARAMETER(attr);
   thread_param_t *params=(thread_param_t *)ortp_new(thread_param_t,1);
   params->func=func;
   params->arg=data;
   *th=(HANDLE)_beginthreadex( nullptr, 0, thread_starter, params, 0, nullptr);
   return 0;
}

i32 WIN_thread_join(ortp_thread_t thread_h, void **unused)
{
   UNREFERENCED_PARAMETER(unused);
   if (thread_h!=nullptr)
   {
      WaitForSingleObject(thread_h, U32_INFINITE_TIMEOUT);
      CloseHandle(thread_h);
   }
   return 0;
}

i32 WIN_cond_init(ortp_cond_t *cond, void *attr)
{
   UNREFERENCED_PARAMETER(attr);
   *cond=CreateEvent(nullptr, FALSE, FALSE, nullptr);
   return 0;
}

i32 WIN_cond_wait(ortp_cond_t* hCond, ortp_mutex_t * hMutex)
{
   //gulp: this is not very atomic ! bug here ?
   WIN_mutex_unlock(hMutex);
   WaitForSingleObject(*hCond, U32_INFINITE_TIMEOUT);
   WIN_single_lock(hMutex);
   return 0;
}

i32 WIN_cond_signal(ortp_cond_t * hCond)
{
   SetEvent(*hCond);
   return 0;
}

i32 WIN_cond_broadcast(ortp_cond_t * hCond)
{
   WIN_cond_signal(hCond);
   return 0;
}

i32 WIN_cond_destroy(ortp_cond_t * hCond)
{
   CloseHandle(*hCond);
   return 0;
}


#if defined(_WIN32_WCE)
#include <time.h>

i32
gettimeofday (struct timeval *tv, void *tz)
{
  u32 timemillis= ::millis::now();
  tv->tv_sec  = timemillis/1000;
  tv->tv_usec = (timemillis - (tv->tv_sec*1000)) * 1000;
  return 0;
}

#else

//CLASS_DECL_AURA i32 gettimeofday (struct timeval *tv, void* tz)
//{
//   UNREFERENCED_PARAMETER(tz);
//   union
//   {
//      i64 ns100; /*time since 1 Jan 1601 in 100ns units */
//      FILETIME fileTime;
//   } now;
//
//   GetSystemTimeAsFileTime (&now.fileTime);
//   tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
//   tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
//   return (0);
//}

#endif

const char *getWinSocketError(i32 error)
{
   static char buf[80];

   switch (error)
   {
      case WSANOTINITIALISED: return "Windows sockets not initialized : call WSAStartup";
      case WSAEADDRINUSE:      return "Local Address already in use";
      case WSAEADDRNOTAVAIL:   return "The specified address is not a valid address for this machine";
      case WSAEINVAL:         return "The socket is already bound to an address.";
      case WSAENOBUFS:      return "Not enough buffers available, too many connections.";
      case WSAENOTSOCK:      return "The descriptor is not a socket.";
      case WSAECONNRESET:      return "Connection reset by peer";

      default :
         sprintf(buf, "Error code : %d", error);
         return buf;
      break;
   }

   return buf;
}

#ifdef _WORKAROUND_MINGW32_BUGS
char * WSAAPI gai_strerror(i32 errnum){
    return (char*)getWinSocketError(errnum);
}
#endif

#endif

#ifndef WIN32

#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/stat.h>

static char *make_pipe_name(const char *name){
   return ortp_strdup_printf("/tmp/%s",name);
}

/* portable named pipes */
ortp_socket_t ortp_server_pipe_create(const char *name){
   struct sockaddr_un sa;
   char *pipename=make_pipe_name(name);
   ortp_socket_t sock;
   sock=socket(AF_UNIX,SOCK_STREAM,0);
   sa.sun_family=AF_UNIX;
   ansi_count_copy(sa.sun_path,pipename,sizeof(sa.sun_path)-1);
   #ifdef WIN32
   _unlink(pipename);/*in case we didn't finished properly previous time */
   #else
   unlink(pipename);/*in case we didn't finished properly previous time */
   #endif
   ortp_free(pipename);
   fchmod(sock,S_IRUSR|S_IWUSR);
   if (bind(sock,(struct sockaddr*)&sa,sizeof(sa))!=0){
      ortp_error("Failed to bind command unix socket: %s",strerror(errno));
      return -1;
   }
   listen(sock,1);
   return sock;
}

ortp_socket_t ortp_server_pipe_accept_client(ortp_socket_t server){
   struct sockaddr_un su;
   socklen_t ssize=sizeof(su);
   ortp_socket_t client_sock=accept(server,(struct sockaddr*)&su,&ssize);
   return client_sock;
}

i32 ortp_server_pipe_close_client(ortp_socket_t client){
   return close(client);
}

i32 ortp_server_pipe_close(ortp_socket_t spipe){
   return close(spipe);
}

ortp_socket_t ortp_client_pipe_connect(const char *name){
   struct sockaddr_un sa;
   char *pipename=make_pipe_name(name);
   ortp_socket_t sock=socket(AF_UNIX,SOCK_STREAM,0);
   sa.sun_family=AF_UNIX;
   ansi_count_copy(sa.sun_path,pipename,sizeof(sa.sun_path)-1);
   ortp_free(pipename);
   if (connect(sock,(struct sockaddr*)&sa,sizeof(sa))!=0){
      close(sock);
      return -1;
   }
   return sock;
}

i32 ortp_pipe_read(ortp_socket_t p, u8 *buf, i32 len){
   return (i32) read(p,buf,len);
}

i32 ortp_pipe_write(ortp_socket_t p, const u8 *buf, i32 len){
   return (i32) write(p,buf,len);
}

i32 ortp_client_pipe_close(ortp_socket_t sock){
   return close(sock);
}

#else

static char *make_pipe_name(const char *name){
   return ortp_strdup_printf("\\\\.\\pipe\\%s",name);
}

static HANDLE event=nullptr;

/* portable named pipes */
ortp_pipe_t ortp_server_pipe_create(const char *name){
   ortp_pipe_t h;
   char *pipename=make_pipe_name(name);
   h=CreateNamedPipeA(pipename,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,PIPE_TYPE_MESSAGE|PIPE_WAIT,1,
                  32768,32768,0,nullptr);
   ortp_free(pipename);
   if (h==INVALID_HANDLE_VALUE){
      ortp_error("Fail to create named pipe %s",pipename);
   }
   if (event==nullptr) event=CreateEvent(nullptr,TRUE,FALSE,nullptr);
   return h;
}


/*this function is a bit complex because we need to wakeup someday
even if nobody connects to the pipe.
ortp_server_pipe_close() makes this function to exit.
*/
ortp_pipe_t ortp_server_pipe_accept_client(ortp_pipe_t server){
   OVERLAPPED ol;
   DWORD undef;
   HANDLE handles[2];
   __memset(&ol,0,sizeof(ol));
   ol.hEvent=CreateEvent(nullptr,TRUE,FALSE,nullptr);
   ConnectNamedPipe(server,&ol);
   handles[0]=ol.hEvent;
   handles[1]=event;
   WaitForMultipleObjects(2,handles,FALSE,U32_INFINITE_TIMEOUT);
   if (GetOverlappedResult(server,&ol,&undef,FALSE)){
      CloseHandle(ol.hEvent);
      return server;
   }
   CloseHandle(ol.hEvent);
   return INVALID_HANDLE_VALUE;
}

i32 ortp_server_pipe_close_client(ortp_pipe_t server){
   return DisconnectNamedPipe(server)==TRUE ? 0 : -1;
}

i32 ortp_server_pipe_close(ortp_pipe_t spipe){
   SetEvent(event);
   //CancelIoEx(spipe,nullptr); /*vista only*/
   return CloseHandle(spipe);
}

ortp_pipe_t ortp_client_pipe_connect(const char *name){
   char *pipename=make_pipe_name(name);
   ortp_pipe_t hpipe = CreateFileA(
         pipename,   // pipe name
         GENERIC_READ |  // read and write access
         GENERIC_WRITE,
         0,              // no sharing
         nullptr,           // default security attributes
         OPEN_EXISTING,  // opens existing pipe
         0,              // default attributes
         nullptr);          // no template file
   ortp_free(pipename);
   return hpipe;
}

i32 ortp_pipe_read(ortp_pipe_t p, u8 *buf, i32 len){
   DWORD ret=0;
   if (ReadFile(p,buf,len,&ret,nullptr))
      return ret;
   /*ortp_error("Could not read from pipe: %s",strerror(GetLastError()));*/
   return -1;
}

i32 ortp_pipe_write(ortp_pipe_t p, const u8 *buf, i32 len)
{

   DWORD ret=0;
   
   if (WriteFile(p,buf,len,&ret,nullptr))
      return ret;
   /*ortp_error("Could not write to pipe: %s",strerror(GetLastError()));*/
   return -1;
}


i32 ortp_client_pipe_close(ortp_pipe_t sock){
   return CloseHandle(sock);
}

#endif

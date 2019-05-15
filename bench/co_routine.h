#ifndef __CO_ROUTINE_H__
#define __CO_ROUTINE_H__

#include <sys/socket.h>
#include <stdint.h>
#include <sys/poll.h>
#include <pthread.h>

//1.struct
struct stCoRoutine_t;
struct stShareStack_t;

struct stCoRoutineAttr_t
{
	int stack_size;
	stShareStack_t*  share_stack;
	stCoRoutineAttr_t()
	{
		stack_size = 128 * 1024;
		share_stack = NULL;
	}
}__attribute__ ((packed));

struct stCoEpoll_t;
typedef int (*pfn_co_eventloop_t)(void *);
typedef void *(*pfn_co_routine_t)( void * );

//2.co_routine
int 	co_create( stCoRoutine_t **co,const stCoRoutineAttr_t *attr,void *(*routine)(void*),void *arg );
void    co_resume( stCoRoutine_t *co );
void    co_yield( stCoRoutine_t *co );
void    co_yield_ct(); //ct = current thread
void    co_release( stCoRoutine_t *co );

stCoRoutine_t *co_self();

int		co_poll( stCoEpoll_t *ctx,struct pollfd fds[], nfds_t nfds, int timeout_ms );
void 	co_eventloop( stCoEpoll_t *ctx,pfn_co_eventloop_t pfn,void *arg );

//3.event
stCoEpoll_t * 	co_get_epoll_ct(); //ct = current thread

//5.hook syscall ( poll/read/write/recv/send/recvfrom/sendto )
void 	co_enable_hook_sys();  
void 	co_disable_hook_sys();  
bool 	co_is_enable_sys_hook();
int     co_accept(int fd, struct sockaddr *addr, socklen_t *len);

//6.share stack
stShareStack_t* co_alloc_sharestack(int iCount, int iStackSize);

//7.log
void co_log_err( const char *fmt,... );

//8. process and thread 
pid_t GetTid();
pid_t GetTid();

#endif


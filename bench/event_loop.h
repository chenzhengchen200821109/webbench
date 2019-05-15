#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include "co_routine.h"
#include "co_routine_inner.h"
#include "inner_pre.h"
#include "server_status.h"
#include <stdlib.h>
#include <functional>
#include <stack>
#include <vector>
#include "coroutine.h"


class EventLoop : public ServerStatus 
{
    public:
        typedef void (*pFunc)(void);
    public:
        typedef std::function<void()> Functor;
	    EventLoop() 
            : ctx(co_get_epoll_ct())
    	{
            tid_ = GetTid(); 
        }

        //Build an EventLoop using an existing event_base object,
        // so we can embed an EventLoop object into the old applications based on libevent
        // NOTE: Be careful to deal with the destructing work of event_base_ and watcher_ objects.
        //explicit EventLoop(struct event_base);
        ~EventLoop()
        {
            //release coroutine 
        }

        // Run the IO Event driving loop forever
        // It MUST be called in the IO Event thread
        void Run();

        void Stop();

        void RunAfter(int seconds, pFunc f);
        void RunEvery(int seconds, pFunc f);
        void StopAfter(int seconds);

        void QueueInLoop(std::unique_ptr<CoRoutine> PtrCo);

        const pid_t tid() const;

        bool IsInLoopThread() const;

        void SetAddr(struct sockaddr_in raddr)
        {
            raddr_ = raddr;
        }
        struct sockaddr_in GetAddr() const
        {
            return raddr_;
        }
        struct Argument
        {
            int seconds;
            pFunc func;
            EventLoop *loop;
        };
private:
    static int HandleEventLoop(void *);
    static void *HandleRunAfter(void *);
    static void *HandleRunEvery(void *);
    static void *HandleStopAfter(void *);
private:
    struct stCoEpoll_t *ctx;
    std::vector<std::unique_ptr<CoRoutine> > coroutines_;
    struct sockaddr_in raddr_;
    pid_t tid_;
};

#endif 

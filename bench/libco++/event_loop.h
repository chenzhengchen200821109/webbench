#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include "co_routine.h"

typedef int (*pEventFunc)(void *);

    class EventLoop {
        public:
	        EventLoop(pEventFunc func, void *arg) : func_(func), ctx(co_get_epoll_ct()), arg_(arg)
    	    {
    
            }

            //Build an EventLoop using an existing event_base object,
            // so we can embed an EventLoop object into the old applications based on libevent
            // NOTE: Be careful to deal with the destructing work of event_base_ and watcher_ objects.
            //explicit EventLoop(struct event_base);
            ~EventLoop()
            {
            
            }

            // Run the IO Event driving loop forever
            // It MUST be called in the IO Event thread
            void Run()
	        {
		        co_eventloop(ctx, func_, arg_);
	        }

            // Stop the event loop
            //void Stop();

            // Reinitialize some data fields after a fork
            //void AfterFork();

            //void RunAfter(double delay_ms, const Functor& f);

            // RunEvery executes Functor f every period interval time.
            //void RunEvery(Duration interval, const Functor& f);

        private:
            pEventFunc func_;
            struct stCoEpoll_t *ctx;
            void *arg_;

    };

#endif 

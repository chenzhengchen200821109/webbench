#ifndef __EVENT_COROUTINE_H__
#define __EVENT_COROUTINE_H__

#include "co_routine.h"
#include "co_routine_inner.h"
#include <functional>

typedef void *(*pFunc)(void *);

    struct stShareStack_t;

    class CoroutineAttr
    {
        public:
            explicit CoroutineAttr() : stack_size(128*1024), share_stack(0)
            {
            
            }
        private:
            int stack_size;
            struct stShareStack_t *share_stack;
    };

    class Coroutine 
    {
        public:
            //typedef std::function<void*(void *)> Functor;
            explicit Coroutine(pFunc func, void *arg) : func_(func), arg_(arg)
            {
                // by default use share stack with 128KB in stack size
                stShareStack_t *share_stack = co_alloc_sharestack(1, 1024 * 128);
                stCoRoutineAttr_t attr;
                attr.stack_size = 0;
                attr.share_stack = share_stack;
                co_create(&co, &attr, func_, arg_);
            }

            explicit Coroutine(const struct stCoRoutineAttr_t *attr, pFunc func, void* arg) : func_(func), arg_(arg)
            {
                co_create(&co, attr_, func_, arg_);
            }

            void Resume()
            {
                co_resume(co);
            }

            void Yield()
            {
                co_yield(co);
            }

            void Yield_Current_Thread()
            {
                co_yield_ct();
            }

            ~Coroutine()
            {
                co_release(co);
            }
        private:
            struct stCoRoutine_t *co;
            const struct stCoRoutineAttr_t* attr_; // coroutine attributes
            pFunc func_;
            void *arg_;
    }; 


#endif 

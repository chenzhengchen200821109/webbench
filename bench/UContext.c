#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "UContext.h"

#define DEFAULT_STACK_SIZE (1024 * (1 << 10))

static Thread_t MainThread;
static int ThreadId = -1;

extern int save_context(UContext_t* from);
extern int restore_context(UContext_t* to);

/* user-thread context switch */
void Thread_switch(Thread_t* from, Thread_t* to)
{
    int ret = save_context(&(from->ucontext));
    if (ret == 0) {
        from->callee = to;
        to->caller = from;
        printf("Thread ID[%d] is restored...\n", to->id);
        restore_context(&(to->ucontext));
    } else {
        /* restored from other threads, just return and continue */
    }
}

/* given a thread ID, create a thread */
int Thread_create_with_ID(Thread_t* thread, int id, thread_handler_t handler)
{
    int stack_size = (1 << 20); // stack size = 1MB
    void* stack_top = malloc(stack_size);
    thread->id = id;
    thread->stack_esp = stack_top + stack_size;
    thread->stack_ebp = stack_top;
    thread->handler = handler;
    memset(&(thread->ucontext), 0, sizeof(UContext_t));
    (thread->ucontext).buffer[esp] = (int)thread->stack_esp;
    (thread->ucontext).buffer[eip] = (int)thread->handler;

    printf("Thread ID[%d] is created...\n", thread->id);
    return id;
}

static void* Thread_Func(Thread_t* thread, void* arg)
{
    // init
    printf("Thread ID[%d] is executing...\n", thread->id);
    if (thread->thread_func)
        thread->thread_func(thread->arg);
    thread->terminated = 1;
    // cleanup
    Thread_exit();

    return NULL;
}

static int UContext_Init(UContext_t* ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    return 0;
}

static int UContext_Make(UContext_t* ctx, ucontext_handler_t pfn, const void* thread, const void* arg)
{
    char* sp = ctx->ss_sp + ctx->ss_size - sizeof(Parameter_t);
    sp = (char *)((unsigned int)sp & ~16L);

    Parameter_t* param = (Parameter_t *)sp;
    param->thread = thread;
    param->arg = arg;

    memset(ctx->buffer, 0, sizeof(ctx->buffer));
    ctx->buffer[esp] = (int)((char *)sp - sizeof(void *));
    ctx->buffer[eip] = (int)(char *)pfn;

    return 0;
}

/* given pthread attributes, create a thread */
int Thread_create(Thread_t* thread, const Thread_Attr_t* attr, thread_handler_t handler, void* arg)
{
    int stack_size;
    void* stack_top;
    if (!thread)
        return -1;
    if (!attr || attr->stacksize <= DEFAULT_STACK_SIZE)
        stack_size = DEFAULT_STACK_SIZE; // default stack size = 1024K
    stack_top = malloc(stack_size);
    thread->id = (++ThreadId);
    thread->stack_esp = stack_top + stack_size;
    thread->stack_ebp = stack_top;
    thread->thread_func = handler;
    thread->arg = arg;
    thread->terminated = 0;
    UContext_Init(&(thread->ucontext));
    thread->ucontext.ss_size = stack_size;
    thread->ucontext.ss_sp = thread->stack_ebp;
    UContext_Make(&(thread->ucontext), (ucontext_handler_t)Thread_Func, thread, arg); 

    printf("Thread ID[%d] is created...\n", thread->id);
    return thread->id;
}

/* thread must be destroyed by user manually */
void Thread_destroy(Thread_t *thread)
{
    if (thread && thread->stack_ebp) {
        //printf("Thread ID[%d] is destroyed...\n", thread->id);
        free(thread->stack_ebp);
    }
}
 
/* start a thread from main */
int Thread_start(Thread_t *thread)
{
    if (!thread) {
        //printf("Thread ID[%d] is restored...\n", thread->id);
        restore_context(&(thread->ucontext));
    }
    return -1; // failed when returns -1.
}

/* save main thread's context and start a new thread */
int Thread_start_with_save(Thread_t* thread)
{
    if (!thread)
        return -1; // failed when returns -1.
    int ret = save_context(&(MainThread.ucontext));
    if (ret == 0) {
        //printf("Thread ID[%d] is restored...\n", thread->id);
        restore_context(&(thread->ucontext));
    }
    else {
        //printf("Main thread is restored...\n");    
    }
    return 0;
}

int Thread_exit()
{
    // what if MainThread invalid
    restore_context(&(MainThread.ucontext));
    return 0;
}

/* restore context of the thread's caller */
void Thread_detach(Thread_t* thread)
{
    printf("Thread ID[%d] is detached...\n", thread->id);
    if (thread) {
        Thread_t* caller = thread->caller;
        if (caller) {
            printf("Thread ID[%d] is restored...\n", caller->id); 
            restore_context(&(caller->ucontext));
        }
        else
            printf("Should never happen but just in case\n");
    }
}



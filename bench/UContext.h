#ifndef _UCONTEXT_H_
#define _UCONTEXT_H_

enum register_t
{
    eip = 0, // program counter
    esp,     // 8 general-purpose registers 
    ebx, 
    ecx,
    edx,
    edi,
    esi,
    ebp,
    eax
};

typedef void* (*ucontext_handler_t)(void* s1, void* s2);
typedef struct Parameter
{
    const void* thread;
    const void* arg;
} Parameter_t;

typedef struct UContext
{
    int buffer[9];
    int ss_size;
    char* ss_sp;
} UContext_t;

typedef void* (*thread_handler_t)(void*);
typedef struct Thread
{
    struct Thread* caller;
    struct Thread* callee;
    int id;
    void* stack_esp;
    void* stack_ebp;
    thread_handler_t handler;
    void* (*thread_func)(void*);
    void* arg;
    int terminated;
    UContext_t ucontext;
} Thread_t;

typedef struct Thread_Attr
{
    int stacksize;
} Thread_Attr_t;

extern int Thread_create_with_ID(Thread_t* thread, int id, thread_handler_t handler);

extern int Thread_create(Thread_t* thread, const Thread_Attr_t* attr, void* (*start_routine)(void *), void* arg);

extern void Thread_destroy(Thread_t* thread);
/* start the new thread without saving context of the main thread */
extern int Thread_start(Thread_t* thread);
/* restore the saved context of main thread, but if we don't have it  */
extern int Thread_exit();
/* just like Thread_start but no saved context of the main thread */
extern int Thread_start_with_save(Thread_t* thread);
extern void Thread_detach(Thread_t* thread);
extern void Thread_switch(Thread_t* from, Thread_t* to);

#endif

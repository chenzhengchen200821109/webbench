#include "event_loop.h"
#include <functional>
#include <stack>

// Run the IO Event driving loop forever
// It MUST be called in the IO Event thread
void EventLoop::Run()
{
    status_.store(kRunning);
    for (size_t i = 0; i < coroutines_.size(); i++) {
        co_resume(coroutines_[i].get()->coroutine);
    }
	co_eventloop(ctx, HandleEventLoop, this);
}

void EventLoop::Stop()
{
    status_.store(kStopped);
}

void EventLoop::RunAfter(int seconds, pFunc f) 
{
    std::unique_ptr<CoRoutine> PtrCo(new CoRoutine);
    struct Argument *arg = (struct Argument *)malloc(sizeof(struct Argument));
    arg->seconds = seconds;
    arg->func = f;
    ::co_create(&(PtrCo.get()->coroutine), NULL, HandleRunAfter, arg);
    QueueInLoop(std::move(PtrCo));
} 

void EventLoop::RunEvery(int seconds, pFunc f) 
{
    std::unique_ptr<CoRoutine> PtrCo(new CoRoutine);
    struct Argument *arg = (struct Argument *)malloc(sizeof(struct Argument));
    arg->seconds = seconds;
    arg->func = f;
    ::co_create(&(PtrCo.get()->coroutine), NULL, HandleRunEvery, arg);
    QueueInLoop(std::move(PtrCo));
} 

void EventLoop::StopAfter(int seconds) 
{
    std::unique_ptr<CoRoutine> PtrCo(new CoRoutine);
    struct Argument *arg = (struct Argument *)malloc(sizeof(struct Argument));
    arg->seconds = seconds;
    arg->loop = this;
    ::co_create(&(PtrCo.get()->coroutine), NULL, HandleStopAfter, arg);
    QueueInLoop(std::move(PtrCo));
} 
void EventLoop::QueueInLoop(std::unique_ptr<CoRoutine> PtrCo)
{
    coroutines_.push_back(std::move(PtrCo));
}

const pid_t EventLoop::tid() const
{
    return tid_;
}

bool EventLoop::IsInLoopThread() const
{
    return tid_ == GetTid();
}

int EventLoop::HandleEventLoop(void *loop)
{
    EventLoop *lp = (EventLoop *)loop;
    if (lp->IsStopped())
        return -1;
    else 
        return 0;
}

void* EventLoop::HandleRunAfter(void *arg)
{
    struct Argument *argument = (struct Argument *)arg;
    int seconds = argument->seconds;
    pFunc func = argument->func;
    free(argument);

    poll(NULL, 0, seconds * 1000);
    func();
    return 0;
}

void* EventLoop::HandleRunEvery(void *arg)
{
    struct Argument *argument = (struct Argument *)arg;
    int seconds = argument->seconds;
    pFunc func = argument->func;
    free(argument);

    for ( ; ; ) {
        poll(NULL, 0, seconds * 1000);
        func();
    }
}

void * EventLoop::HandleStopAfter(void *arg)
{
    struct Argument *argument = (struct Argument *)arg;
    int seconds = argument->seconds;
    EventLoop *lp = argument->loop;

    poll(NULL, 0, seconds * 1000);
    lp->Stop();
    return 0;
}

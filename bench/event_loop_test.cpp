#include "event_loop.h"

EventLoop* lp;

void timer_routine(void)
{
    printf("after a long time\n");
    lp->Stop();
}

void period_routine(void)
{
    printf("each a long time\n");
}


int main()
{
    EventLoop loop;
    
    lp = &loop;
    loop.RunAfter(30, timer_routine);
    loop.RunEvery(10, period_routine);
    loop.Run();

    printf("Bye Bye\n");
    return 0;
}

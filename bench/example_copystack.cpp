#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include "coctx.h"
#include "co_routine.h"
#include "co_routine_inner.h"

void* RoutineFunc(void* args)
{
	co_enable_hook_sys();
	int* routineid = (int*)args;
	while (true)
	{
		char sBuff[128];
		sprintf(sBuff, "from routineid %d stack addr %p\n", *routineid, sBuff);

		printf("%s", sBuff);
		poll(NULL, 0, 1000); //sleep 1ms
	}
	return NULL;
}

int main()
{
	// 分配一个共享栈，栈的大小为128KB
	stShareStack_t* share_stack= co_alloc_sharestack(1, 1024 * 128);
	stCoRoutineAttr_t attr;
	attr.stack_size = 0;
	attr.share_stack = share_stack;

	stCoRoutine_t* co[2];
	int routineid[2];
	for (int i = 0; i < 2; i++)
	{
		routineid[i] = i;
		co_create(&co[i], &attr, RoutineFunc, routineid + i);
		// 从这里切换到第一个协程，然后从第一个协程又切换回来
		// 后再次切换到第二个协程，然后再切换回来
		co_resume(co[i]);
	}
	co_eventloop(co_get_epoll_ct(), NULL, NULL);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include "event_loop.h"
#include "event_coroutine.h"

void* RoutineFunc(void* args)
{
	//co_enable_hook_sys();
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
    EventLoop loop(0, 0);
	Coroutine* co[2];
	int routineid[2];

	for (int i = 0; i < 2; i++)
	{
		routineid[i] = i;
        co[i] = new Coroutine(RoutineFunc, routineid+i);
		co[i]->Resume();
	}
	
    loop.Run();
	return 0;
}

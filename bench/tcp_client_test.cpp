#include "event_loop.h"
#include "tcp_client.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string>

int main(int argc,char *argv[])
{
	int proccnt = atoi( argv[4] ); //进程数量

	// 为什么要忽略SIGPIPE信号？
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction( SIGPIPE, &sa, NULL );

    EventLoop loop;

	for(int k=0;k<proccnt;k++)
	{

		pid_t pid = fork();
		if( pid > 0 ) {
			continue;
		} else if( pid < 0 ) {
			break;
		}
        TCPClient cli(&loop, argv[1], atoi(argv[2]), atoi(argv[3]));
        cli.Start();
        loop.Run();

		exit(0);
	}
    
    // wait for all child process
    wait(NULL);
	return 0;
}
/*./example_echosvr 127.0.0.1 10000 100 50*/

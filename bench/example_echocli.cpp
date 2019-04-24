#include "co_routine.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stack>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;
struct stEndPoint
{
	char *ip;
	unsigned short int port;
};

/* */
static void SetAddr(const char *pszIP,const unsigned short shPort,struct sockaddr_in &addr)
{
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(shPort);
	int nIP = 0;
	if( !pszIP || '\0' == *pszIP   
			|| 0 == strcmp(pszIP,"0") || 0 == strcmp(pszIP,"0.0.0.0") 
			|| 0 == strcmp(pszIP,"*"))
	{
        /* convert from host byte to network byte order */
		nIP = htonl(INADDR_ANY);
	}
	else
	{
		nIP = inet_addr(pszIP);
	}
	addr.sin_addr.s_addr = nIP;

}

static int iSuccCnt = 0;
static int iFailCnt = 0;
static int iTime = 0;
static pid_t iPid = 0;

void AddSuccCnt()
{
    if (iPid == 0)
        iPid = getpid();
    /* returns the time as the number of seconds since the Epoch */
	int now = time(NULL);
	if (now >iTime)
	{
		printf("time %d PID[%d] TID[%d] Succ Cnt %d Fail Cnt %d\n", iTime, iPid, GetTid(), iSuccCnt, iFailCnt);
		iTime = now;
		iSuccCnt = 0;
		iFailCnt = 0;
	}
	else
	{
		iSuccCnt++;
	}
}
void AddFailCnt()
{
    if (iPid == 0)
        iPid = getpid();
	int now = time(NULL);
	if (now >iTime)
	{
		printf("time %d PID[%d] TID[%d] Succ Cnt %d Fail Cnt %d\n", iTime, iPid, GetTid(), iSuccCnt, iFailCnt);
		iTime = now;
		iSuccCnt = 0;
		iFailCnt = 0;
	}
	else
	{
		iFailCnt++;
	}
}

static void *readwrite_routine( void *arg )
{

	co_enable_hook_sys();

	stEndPoint *endpoint = (stEndPoint *)arg;
	char str[8]="sarlmol";
	char buf[ 1024 * 16 ];
	int fd = -1;
	int ret = 0;
	for(;;)
	{
		if ( fd < 0 )
		{
			fd = socket(PF_INET, SOCK_STREAM, 0);
			struct sockaddr_in addr;
			SetAddr(endpoint->ip, endpoint->port, addr);
			ret = connect(fd,(struct sockaddr*)&addr,sizeof(addr));

			// EALREADY -- The socket is nonblocking and a previous connection attempt has not yet been completed.
			// EINPROGRESS -- The socket is nonblocking and the connection cannot be completed immediately.  
			//                It is possible to select(2) or poll(2) for completion by selecting the socket for writing.  
			//                After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level 
			//                SOL_SOCKET  to  determine	whether connect()  completed	successfully  (SO_ERROR is zero) 
			//                or unsuccessfully (SO_ERROR is one of the usual error codes listed here, explaining the reason
			//   			  for the failure).
			if ( errno == EALREADY || errno == EINPROGRESS )
			{       
				struct pollfd pf = { 0 };
				pf.fd = fd;
				pf.events = (POLLOUT|POLLERR|POLLHUP);
				co_poll( co_get_epoll_ct(),&pf,1,200); 
				//check connect
				int error = 0;
				uint32_t socklen = sizeof(error);
				errno = 0;
				ret = getsockopt(fd, SOL_SOCKET, SO_ERROR,(void *)&error,  &socklen);
				if ( ret == -1 ) 
				{       
					//printf("getsockopt ERROR ret %d %d:%s\n", ret, errno, strerror(errno));
					close(fd);
					fd = -1;
					AddFailCnt();
					continue;
				}       
				if ( error ) 
				{       
					errno = error;
					//printf("connect ERROR ret %d %d:%s\n", error, errno, strerror(errno));
					close(fd);
					fd = -1;
					AddFailCnt();
					continue;
				}       
			} 
	  			
		}
		
		ret = write( fd,str, 8);
		if ( ret > 0 )
		{
			ret = read( fd,buf, sizeof(buf) );
			if ( ret <= 0 )
			{
				//printf("co %p read ret %d errno %d (%s)\n",
				//		co_self(), ret,errno,strerror(errno));
				close(fd);
				fd = -1; // 有错误发生要重新建立连接
				AddFailCnt();
			}
			else
			{
				//printf("echo %s fd %d\n", buf,fd);
				AddSuccCnt();
			}
		}
		else
		{
			//printf("co %p write ret %d errno %d (%s)\n",
			//		co_self(), ret,errno,strerror(errno));
			close(fd);
			fd = -1; // 有错误发生要重新建立连接
			AddFailCnt();
		}
	}
	return 0;
}

int main(int argc,char *argv[])
{
	stEndPoint endpoint;
	endpoint.ip = argv[1];
	endpoint.port = atoi(argv[2]);
	int cnt = atoi( argv[3] ); //协程数量
	int proccnt = atoi( argv[4] ); //进程数量

	// 为什么要忽略SIGPIPE信号？
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction( SIGPIPE, &sa, NULL );

	for(int k=0;k<proccnt;k++)
	{

		pid_t pid = fork();
		if( pid > 0 ) // parent process
		{
			continue;
		}
		else if( pid < 0 )
		{
			break;
		}
		// child process
		for(int i=0;i<cnt;i++)
		{
			stCoRoutine_t *co = 0;
			co_create( &co,NULL,readwrite_routine, &endpoint);
			co_resume( co );
		}
		co_eventloop( co_get_epoll_ct(),0,0 );

		exit(0);
	}
    
    // wait for all child process
    wait(NULL);
	return 0;
}
/*./example_echosvr 127.0.0.1 10000 100 50*/

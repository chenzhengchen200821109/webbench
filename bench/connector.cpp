#include "inner_pre.h"
#include "connector.h"
#include "event_loop.h"
#include "co_routine.h"
#include "sockets.h"
#include "utility.h"
#include <iostream>
#include <stdio.h>
#include <time.h>

int iSuccCnt;
int iFailCnt;
int iConnect;
size_t iBytes;

static char input_buffer[1024 * 1024 * 4];
//static char str[1024] = "GET / HTTP/1.1\r\nConnection: keep-alive\r\nUser-Agent: WebBench 1.5\r\nHost: www.baidu.com\r\n\r\n";
extern char request[];

Connector::Connector(EventLoop* loop, const char *ip, const unsigned short port) 
    : loop_(loop)
    , ip_(ip)
    , port_(port)
{
    //DLOG_TRACE << "raddr=" << remote_addr_;
    //std::cout << "Connector::Connector()" << std::endl;
    PtrCo.reset(new CoRoutine);
    /* create a coroutine */
    //::co_create(&connect_co, NULL, HandleConnect, loop);
    ::co_create(&(PtrCo.get()->coroutine), NULL, HandleConnect, loop);
}

Connector::~Connector()
{
    //DLOG_TRACE;
    //std::cout << "Connector::~Connector()" << std::endl;
}

void Connector::Connect()
{
    SetAddr(ip_, port_, raddr_);
    loop_->SetAddr(raddr_);
}

void Connector::Start()
{
    //DLOG_TRACE << "Try to connect " << remote_addr_ << " status=" << StatusToString();
    //std::cout << "Connector::Start()" << std::endl;
    loop_->QueueInLoop(std::move(PtrCo));
}

void Connector::SetAddr(const char *pszIP, const unsigned short shPort, struct sockaddr_in& addr)
{
    bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(shPort);
	int nIP = 0;
	if( !pszIP || '\0' == *pszIP   
			|| 0 == strcmp(pszIP,"0") || 0 == strcmp(pszIP,"0.0.0.0") 
			|| 0 == strcmp(pszIP,"*") 
	  )
	{
		nIP = htonl(INADDR_ANY);
	}
	else
	{
		nIP = inet_addr(pszIP);
	}
	addr.sin_addr.s_addr = nIP;
}

extern int http10;
extern int force;

void* Connector::HandleConnect(void *loop)
{
    co_enable_hook_sys();
    EventLoop *lp = (EventLoop *)loop;
    struct sockaddr_in raddr = lp->GetAddr();
    int fd = -1;
    int ret = 0;
    size_t rlen = 0;

	for( ; ; ) {
        //printf("iSuccCnt = %d, iFailCnt = %d, iConnect = %d, iBytes = %d\n", iSuccCnt, iFailCnt, iConnect, iBytes);
        if (fd < 0) {
            fd = socket(AF_INET, SOCK_STREAM, 0);
            ret = connect(fd, (struct sockaddr *)&raddr, sizeof(raddr));
		    // EALREADY -- The socket is nonblocking and a previous connection attempt has not yet been completed.
		    // EINPROGRESS -- The socket is nonblocking and the connection cannot be completed immediately.  
		    //                It is possible to select(2) or poll(2) for completion by selecting the socket for writing.  
		    //                After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level 
		    //                SOL_SOCKET  to  determine	whether connect()  completed	successfully  (SO_ERROR is zero) 
		    //                or unsuccessfully (SO_ERROR is one of the usual error codes listed here, explaining the reason
		    //   			  for the failure).
            //perror("connect failed");
		    if (errno == EALREADY || errno == EINPROGRESS) {       
			    struct pollfd pf = { 0 };
			    pf.fd = fd;
			    pf.events = (POLLOUT|POLLERR|POLLHUP);
			    co_poll(co_get_epoll_ct(), &pf, 1, 200); 
			    //check connect
			    int error = 0;
			    uint32_t socklen = sizeof(error);
			    errno = 0;
			    ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&error, &socklen);
			    if (ret == -1) {       
                    close(fd);
                    fd = -1;
                    iFailCnt++;
                    continue;
			    }       
			    if (error) {       
			        errno = error;
                    close(fd);
                    fd = -1;
                    iFailCnt++;
                    continue;
			    }
		    }
            iConnect++;
        }

        rlen = strlen(request);
        ret = write(fd, request, rlen);
        if (ret > 0) {
            if (ret != rlen) {
                //wait  
                struct pollfd pf = { 0 };
                pf.fd = fd;
                pf.events = (POLLIN|POLLERR|POLLHUP);
                co_poll(co_get_epoll_ct(), &pf, 1, 200);
            }

            if (http10 == 0) {
                if (shutdown(fd, SHUT_WR)) {
                    iFailCnt++;
                    close(fd);
                    fd = -1;
                    continue;
                }
            }

            if (force == 0) {
        tryagain:
                ret = read(fd, input_buffer, sizeof(input_buffer));
                if (ret < 0) {
                    //perror("read failed");
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        struct pollfd pf = { 0 };
                        pf.fd = fd;
                        pf.events = (POLLIN|POLLERR|POLLHUP);
                        co_poll(co_get_epoll_ct(), &pf, 1, 200);
                        goto tryagain;
                    } 
                    else {
                        close(fd);
                        fd = -1;
                        iFailCnt++;
                    }
                }
                else if (ret == 0) { //remote peer shutdown
                    close(fd);
                    fd = -1;
                }
                else {
                    //printf("%s\n", input_buffer);
                    //close(fd);
                    //fd = -1;
                    iSuccCnt++;
                    iBytes += ret;
                    goto tryagain;
                }
            }
        } else {
            //perror("write failed");
            close(fd);
            fd = -1;
            iFailCnt++;
        }
	}
    return 0;
}


#include "webbench.h"
#include "socket.h"
#include <stdio.h>
#include <unistd.h>
#include "UContext.h"
#include <sys/time.h>

extern int force;
extern int proxyport;
extern char* proxyhost;
extern char* request;


/* 线程池或者协程池 */
int thread_index = 0;
Thread_t thread_pool[THREAD_POOL_SIZE];

void* thread_func(void* arg)
{
    int size = THREAD_POOL_SIZE;
    int count = 1;
    int sockfd;

    while (count > 0) {
        int temp = (thread_index % size);
        thread_index = ((temp + 1) % size);
        //printf("Thread ID[%d] is doing stuff with stack top[%p] curr:%d:next:%d\n", thread_pool[temp].id, thread_pool[temp].stack_esp, temp, thread_index);
        sockfd = Send_Request(proxyhost, proxyport, request);
	    Thread_switch(&thread_pool[temp], &thread_pool[thread_index]);  //switch to thread B
        /* when switch back, exectution restarts from here */
        count--;
    }

    if (force == 1) {
        Thread_exit();
        return NULL;
    }

    count = 1;
    while (count > 0) {
        int temp = (thread_index % size);
        thread_index = ((temp + 1) % size);
        Read_Response(sockfd);
        Thread_switch(&thread_pool[temp], &thread_pool[thread_index]);
        count--;
    }

    Thread_exit();
    return NULL;
}

void Create_Clients(int clients)
{
    int size = THREAD_POOL_SIZE;
    int i;
    for (i = 0; i < size; i++) {
        Thread_create_with_ID(&thread_pool[i], i, thread_func);
    }

}

int Send_Request(char* name, int port, char* req)
{
    size_t rlen;
    int sockfd;

    if ((sockfd = Socket(name, port)) < 0) {
        close(sockfd); 
        return -1;
    }

    if (rlen != write(sockfd, req, rlen)) {
        close(sockfd);
        return -1;
    }

    return 0;
}

char* Read_Response(int sockfd)
{
    size_t i;
    char buf[1500];

    if ((i = read(sockfd, buf, 1500)) < 0) {
        close(sockfd);
        return NULL;
    }

    return NULL;
}

void Destroy_Clients()
{
    int size = THREAD_POOL_SIZE;
    int i;
    for (i = 0; i < size; i++) {
        Thread_destroy(&thread_pool[i]);
    }
}

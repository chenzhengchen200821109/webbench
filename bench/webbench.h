#ifndef WEBBENCH_H
#define WEBBENCH_H

#define MAXHOSTNAMELEN 1048
#define THREAD_POOL_SIZE 1000000

extern void Create_Clients(int clients);
extern void* thread_func(void* arg);

extern int Send_Request(char* hostname, int port, char* req);
extern char* Read_Response(int sockfd);

extern void Destroy_Clients();


#endif 

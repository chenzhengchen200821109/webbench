#include "socket.h"
#include <stdio.h>

#define FD_NUM 500

int fds[FD_NUM];
int success;
int failed;

char* server = "www.baidu.com";
int port = 80;

int main()
{
    int num = FD_NUM;
    int i;
    int sockfd;

    for (i = 0; i < num; i++) {
        if ((sockfd = Socket(server, port)) != -1)
            fds[i] = sockfd;
        else
            fds[i] = -1;
    }

    for (i = 0; i < num; i++) {
        printf("sockfd = %d\n", fds[i]);
        if (fds[i] == -1)
            failed++;
        else {
            close(fds[i]);
            success++;
        }
    }

    printf("success = %d, failed = %d\n", success, failed);

    return 0;
}

#include "socket.h"
#include <errno.h>

extern int h_errno;
struct sockaddr_in ad;
int flag = 0;

int Socket(const char* host, int clientPort)
{
    int sockfd;
    unsigned long inaddr;
    //struct sockaddr_in ad;
    struct hostent *hp;
    
    if (flag == 0) {
        memset(&ad, 0, sizeof(ad));
        ad.sin_family = AF_INET;
        /*
         * inet_addr() function converts the Internet host address from IPv4 numbers-and-dots
         * into binary data in network byte order. If the input is invalid, INADDR_NONE
         * is returned.
         */
        inaddr = inet_addr(host);
        if (inaddr != INADDR_NONE)
            memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
        else {
            /*
             * gethostbyname() function returns a structure of type hostent for the given host name
             */
            hp = gethostbyname(host);
            if (hp == NULL) {
                herror("gethostbyname error");
                return -1;
            }
            memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
        }
        ad.sin_port = htons(clientPort);
        flag = 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *)&ad, sizeof(ad)) < 0) {
        perror("connect failed");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int UnBlock_Connect(int sockfd)
{
    int ret;

    ret = connect(sockfd, (struct sockaddr *)&ad, sizeof(ad));
    if (ret == 0)
        return 0;
    else if (errno != EINPROGRESS)
        return -1;
    else
        return EINPROGRESS;
}

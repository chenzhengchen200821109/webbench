#include "socket.h"

extern int h_errno;

int Socket(const char* host, int clientPort)
{
    int sockfd;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;
    
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
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return sockfd;
    if (connect(sockfd, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return -1;
    return sockfd;
}


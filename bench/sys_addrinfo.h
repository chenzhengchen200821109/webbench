// Copy from Chromium project.

// This is a convenience header to pull in the platform-specific headers
// that define at least:
//
//     struct addrinfo
//     struct sockaddr*
//     getaddrinfo()
//     freeaddrinfo()
//     AI_*
//     AF_*
//
// Prefer including this file instead of directly writing the #if / #else,
// since it avoids duplicating the platform-specific selections.

#ifndef __SYS_ADDRINFO_H__
#define __SYS_ADDRINFO_H__

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // for TCP_NODELAY
#include <sys/socket.h>
#include <arpa/inet.h>

#endif

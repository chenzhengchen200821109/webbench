#include "sockets.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

static const std::string empty_string;

int SetNonBlockingSocket(int fd)
{
    int flags;
	if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
		return -1;
	}
	if (!(flags & O_NONBLOCK)) {
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK | O_NDELAY) == -1) {
			return -1;
		}
	}
    return 0;
}

int CreateNonblockingSocket() 
{
    int serrno = 0;

    /* Create listen socket */
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        //serrno = errno;
        //LOG_ERROR << "socket error " << strerror(serrno);
        return -1;
    }

    if (SetNonBlockingSocket(fd) < 0) {
        goto out;
    }

    SetKeepAlive(fd, true);
    SetReuseAddr(fd);
    SetReusePort(fd);
    return fd;
out:
    close(fd);
    return -1;
}

int CreateUDPServer(int port) 
{
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        //int serrno = errno;
        //LOG_ERROR << "socket error " << strerror(serrno);
        return -1;
    }
    SetReuseAddr(fd);
    SetReusePort(fd);

    std::string addr = std::string("0.0.0.0:") + std::to_string(port);
    struct sockaddr_storage local = ParseFromIPPort(addr.c_str());
    if (::bind(fd, (struct sockaddr*)&local, sizeof(struct sockaddr))) {
        //int serrno = errno;
        //LOG_ERROR << "socket bind error=" << serrno << " " << strerror(serrno);
        return -1;
    }

    return fd;
}

bool ParseFromIPPort(const char* address, struct sockaddr_storage& ss) 
{
    memset(&ss, 0, sizeof(ss));
    std::string host;
    int port;
    if (!SplitHostPort(address, host, port)) {
        return false;
    }

    short family = AF_INET;
    auto index = host.find(':');
    if (index != std::string::npos) {
        family = AF_INET6;
    }

    struct sockaddr_in* addr = sockaddr_in_cast(&ss);
    int rc = inet_pton(family, host.data(), &addr->sin_addr);
    if (rc == 0) {
        //LOG_INFO << "ParseFromIPPort evutil_inet_pton (AF_INET '" << host.data() << "', ...) rc=0. " << host.data() << " is not a valid IP address. Maybe it is a hostname.";
        return false;
    } else if (rc < 0) {
        int serrno = errno;
        if (serrno == 0) {
            //LOG_INFO << "[" << host.data() << "] is not a IP address. Maybe it is a hostname.";
        } else {
            //LOG_WARN << "ParseFromIPPort evutil_inet_pton (AF_INET, '" << host.data() << "', ...) failed : " << strerror(serrno);
        }
        return false;
    }

    addr->sin_family = family;
    addr->sin_port = htons(port);

    return true;
}

bool SplitHostPort(const char* address, std::string& host, int& port) 
{
    std::string a = address;
    if (a.empty()) {
        return false;
    }

    size_t index = a.rfind(':');
    if (index == std::string::npos) {
        //LOG_ERROR << "Address specified error <" << address << ">. Cannot find ':'";
        return false;
    }

    if (index == a.size() - 1) {
        return false;
    }

    port = ::atoi(&a[index + 1]);

    host = std::string(address, index);
    if (host[0] == '[') {
        if (*host.rbegin() != ']') {
            //LOG_ERROR << "Address specified error <" << address << ">. '[' ']' is not pair.";
            return false;
        }

        // trim the leading '[' and trail ']'
        host = std::string(host.data() + 1, host.size() - 2);
    }

    // Compatible with "fe80::886a:49f3:20f3:add2]:80"
    if (*host.rbegin() == ']') {
        // trim the trail ']'
        host = std::string(host.data(), host.size() - 1);
    }

    return true;
}

struct sockaddr_storage GetLocalAddr(int sockfd) 
{
    struct sockaddr_storage laddr;
    memset(&laddr, 0, sizeof laddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof laddr);
    if (::getsockname(sockfd, sockaddr_cast(&laddr), &addrlen) < 0) {
        //LOG_ERROR << "GetLocalAddr:" << strerror(errno);
        memset(&laddr, 0, sizeof laddr);
    }

    return laddr;
}

std::string ToIPPort(const struct sockaddr_storage* ss) 
{
    std::string saddr;
    int port = 0;

    if (ss->ss_family == AF_INET) {
        struct sockaddr_in* addr4 = const_cast<struct sockaddr_in*>(sockaddr_in_cast(ss));
        char buf[INET_ADDRSTRLEN] = {};
        const char* addr = inet_ntop(ss->ss_family, &addr4->sin_addr, buf, INET_ADDRSTRLEN);
        if (addr) {
            saddr = addr;
        }

        port = ntohs(addr4->sin_port);
    } else if (ss->ss_family == AF_INET6) {
        struct sockaddr_in6* addr6 = const_cast<struct sockaddr_in6*>(sockaddr_in6_cast(ss));
        char buf[INET6_ADDRSTRLEN] = {};
        const char* addr = inet_ntop(ss->ss_family, &addr6->sin6_addr, buf, INET6_ADDRSTRLEN);
        if (addr) {
            saddr = std::string("[") + addr + "]";
        }
        port = ntohs(addr6->sin6_port);
    } else {
        //LOG_ERROR << "unknown socket family connected";
        return empty_string;
    }

    if (!saddr.empty()) {
        saddr.append(":", 1).append(std::to_string(port));
    }

    return saddr;
}

std::string ToIPPort(const struct sockaddr* ss) 
{
    return ToIPPort(sockaddr_storage_cast(ss));
}

std::string ToIPPort(const struct sockaddr_in* ss) 
{
    return ToIPPort(sockaddr_storage_cast(ss));
}

std::string ToIP(const struct sockaddr* s) 
{
    auto ss = sockaddr_storage_cast(s);
    if (ss->ss_family == AF_INET) {
        struct sockaddr_in* addr4 = const_cast<struct sockaddr_in*>(sockaddr_in_cast(ss));
        char buf[INET_ADDRSTRLEN] = {};
        const char* addr = ::inet_ntop(ss->ss_family, &addr4->sin_addr, buf, INET_ADDRSTRLEN);
        if (addr) {
            return std::string(addr);
        }
    } else if (ss->ss_family == AF_INET6) {
        struct sockaddr_in6* addr6 = const_cast<struct sockaddr_in6*>(sockaddr_in6_cast(ss));
        char buf[INET6_ADDRSTRLEN] = {};
        const char* addr = ::inet_ntop(ss->ss_family, &addr6->sin6_addr, buf, INET6_ADDRSTRLEN);
        if (addr) {
            return std::string(addr);
        }
    } else {
        //LOG_ERROR << "unknown socket family connected";
    }

    return empty_string;
}

void SetKeepAlive(int fd, bool on) 
{
    int optval = on ? 1 : 0;
    int rc = ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                          reinterpret_cast<const char*>(&optval), static_cast<socklen_t>(sizeof optval));
    if (rc != 0) {
        //int serrno = errno;
        //LOG_ERROR << "setsockopt(SO_KEEPALIVE) failed, errno=" << serrno << " " << strerror(serrno);
    }
}

void SetReuseAddr(int fd) 
{
    int optval = 1;
    int rc = ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                          reinterpret_cast<const char*>(&optval), static_cast<socklen_t>(sizeof optval));
    if (rc != 0) {
        //int serrno = errno;
        //LOG_ERROR << "setsockopt(SO_REUSEADDR) failed, errno=" << serrno << " " << strerror(serrno);
    }
}

void SetReusePort(int fd) 
{
#ifdef SO_REUSEPORT
    int optval = 1;
    int rc = ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
                          reinterpret_cast<const char*>(&optval), static_cast<socklen_t>(sizeof optval));
    //LOG_INFO << "setsockopt SO_REUSEPORT fd=" << fd << " rc=" << rc;
    if (rc != 0) {
        //int serrno = errno;
        //LOG_ERROR << "setsockopt(SO_REUSEPORT) failed, errno=" << serrno << " " << strerror(serrno);
    }
#endif
}


void SetTCPNoDelay(int fd, bool on) 
{
    int optval = on ? 1 : 0;
    int rc = ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                          reinterpret_cast<const char*>(&optval), static_cast<socklen_t>(sizeof optval));
    if (rc != 0) {
        //int serrno = errno;
        //LOG_ERROR << "setsockopt(TCP_NODELAY) failed, errno=" << serrno << " " << strerror(serrno);
    }
}

void HostToIP(const char* host, char* IP)
{
    struct addrinfo *result = NULL;                                                
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;
  
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
 
    int ret = getaddrinfo(host, 0, &hints, &result);
    if (ret != 0) {
        perror("getaddrinfo failed");
        return;
    }
    else {
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            if (ptr->ai_family == AF_INET) {
                struct sockaddr_in * ipv4 = (struct sockaddr_in *)ptr->ai_addr;
                char *src = inet_ntoa(ipv4->sin_addr);
                strncpy(IP, src, strlen(src));
                //printf("IP = %s\n", IP);
                //printf("IPv4 address is %s\n", inet_ntoa(ipv4->sin_addr));
                break;
            }
        }
    }
}

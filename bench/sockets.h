#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // for TCP_NODELAY
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

extern int SetNonBlockingSocket(int fd);
extern int CreateNonblockingSocket();
extern int CreateUDPServer(int port);
extern void SetKeepAlive(int fd, bool on);
extern void SetReuseAddr(int fd);
extern void SetReusePort(int fd);
extern void SetTCPNoDelay(int fd, bool on);
extern std::string ToIPPort(const struct sockaddr_storage* ss);
extern std::string ToIPPort(const struct sockaddr* ss);
extern std::string ToIPPort(const struct sockaddr_in* ss);
extern std::string ToIP(const struct sockaddr* ss);

extern void HostToIP(const char* host, char* IP);


// @brief Parse a literal network address and return an internet protocol family address
// @param[in] address - A network address of the form "host:port" or "[host]:port"
// @return bool - false if parse failed.
extern bool ParseFromIPPort(const char* address, struct sockaddr_storage& ss);

inline struct sockaddr_storage ParseFromIPPort(const char* address) 
{
    struct sockaddr_storage ss;
    bool rc = ParseFromIPPort(address, ss);
    if (rc) {
        return ss;
    } else {
        memset(&ss, 0, sizeof(ss));
        return ss;
    }
}

// @brief Splits a network address of the form "host:port" or "[host]:port"
//  into host and port. A literal address or host name for IPv6
// must be enclosed in square brackets, as in "[::1]:80" or "[ipv6-host]:80"
// @param[in] address - A network address of the form "host:port" or "[ipv6-host]:port"
// @param[out] host -
// @param[out] port - the port in local machine byte order
// @return bool - false if the network address is invalid format
extern bool SplitHostPort(const char* address, std::string& host, int& port);

extern struct sockaddr_storage GetLocalAddr(int sockfd);

inline bool IsZeroAddress(const struct sockaddr_storage* ss) {
    const char* p = reinterpret_cast<const char*>(ss);
    for (size_t i = 0; i < sizeof(*ss); ++i) {
        if (p[i] != 0) {
            return false;
        }
    }
    return true;
}

template<typename To, typename From>
inline To implicit_cast(From const& f) {
    return f;
}

inline const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) 
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

inline struct sockaddr* sockaddr_cast(struct sockaddr_in* addr) 
{
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

inline struct sockaddr* sockaddr_cast(struct sockaddr_storage* addr) 
{
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

inline const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr) 
{
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

inline struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr) 
{
    return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
}

inline struct sockaddr_in* sockaddr_in_cast(struct sockaddr_storage* addr) 
{
    return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
}

inline struct sockaddr_in6* sockaddr_in6_cast(struct sockaddr_storage* addr) 
{
    return static_cast<struct sockaddr_in6*>(implicit_cast<void*>(addr));
}

inline const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr_storage* addr) 
{
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

inline const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr_storage* addr) 
{
    return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

inline const struct sockaddr_storage* sockaddr_storage_cast(const struct sockaddr* addr) 
{
    return static_cast<const struct sockaddr_storage*>(implicit_cast<const void*>(addr));
}

inline const struct sockaddr_storage* sockaddr_storage_cast(const struct sockaddr_in* addr) 
{
    return static_cast<const struct sockaddr_storage*>(implicit_cast<const void*>(addr));
}

inline const struct sockaddr_storage* sockaddr_storage_cast(const struct sockaddr_in6* addr) 
{
    return static_cast<const struct sockaddr_storage*>(implicit_cast<const void*>(addr));
}

#endif

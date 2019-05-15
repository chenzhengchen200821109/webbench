#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__ 

#include "inner_pre.h"
#include "event_loop.h"
#include "connector.h"
#include <vector>

class Connector;
// We can use this class to create a TCP client.
// The typical usage is :
//      1. Create a TCPClient object
//      2. Set the message callback and connection callback
//      3. Call TCPClient::Connect() to try to establish a connection with remote server
//      4. Use TCPClient::Send(...) to send messages to remote server
//      5. Handle the connection and messages in callbacks
//      6. Call TCPClient::Disonnect() to disconnect from remote server
//
class TCPClient 
{
    public:
        TCPClient(EventLoop* loop, const char *ip, const unsigned short port, int NumCo);
        ~TCPClient(); 
        void Start();
    private: 
        void Connect();
        EventLoop* loop_;
        Connector connector_;
        std::vector<Connector> connectors_;
        int NumCo_;
};

#endif

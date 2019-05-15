#include <atomic>
#include "inner_pre.h"
#include "tcp_client.h"
#include "connector.h"
#include <errno.h>
#include <stdio.h>
#include <iostream>

TCPClient::TCPClient(EventLoop* loop, const char *ip, const unsigned short port, int NumCo)
    : loop_(loop)
    , connector_(loop, ip, port)
    , NumCo_(NumCo)
{
    //DLOG_TRACE << "remote addr=" << raddr;
    //std::cout << "TCPClient::TCPClient()" << std::endl;
    for (int i = 0; i < (NumCo_ - 1); i++) {
        Connector con(loop, ip, port);
        connectors_.push_back(std::move(con));
    }
}

TCPClient::~TCPClient() 
{
    //DLOG_TRACE;
    //std::cout << "TCPClient::~TCPClient()" << std::endl;
}

void TCPClient::Start()
{
    Connect();
    for (size_t i = 0; i < connectors_.size(); i++) {
        connectors_[i].Start();
    }
}

void TCPClient::Connect() 
{
    //LOG_INFO << "remote_addr=" << remote_addr();
    //std::cout << "TCPClient::Connect()" << std::endl;
    connector_.Connect();
    connector_.Start();
}

#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "inner_pre.h"
#include <string>
#include <memory> 
#include "coroutine.h"

class EventLoop;

class Connector
{
    public:
        Connector(EventLoop* loop, const char* ip, const unsigned short port); 
        ~Connector();
        void Start();
        void Connect();
        //void Cancel();
        Connector(Connector&& con) : loop_(con.loop_), ip_(con.ip_), port_(con.port_), PtrCo(std::move(con.PtrCo))
        {

        }
        Connector& operator=(Connector&& con)
        {
            if (this != &con)
                PtrCo = std::move(con.PtrCo);
            return *this;
        }
        Connector(const Connector&) = delete;
        Connector& operator=(const Connector&) = delete;
    private:
        static void* HandleConnect(void *);
        //static int HandleCancel(void *conn);
        void SetAddr(const char *, const unsigned short, struct sockaddr_in&);
    private:
        //enum Status { kDisconnected, kConnected };
        //Status status_;
        EventLoop* loop_;
        const char* ip_;
        const unsigned short port_;
        std::unique_ptr<CoRoutine> PtrCo;
        struct sockaddr_in raddr_;
};

#endif

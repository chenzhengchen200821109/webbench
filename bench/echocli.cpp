#include "tcp_client.h"
#include "buffer.h"
#include "tcp_cnn.h"

// read 
void OnMessage(int fd, char *buf) 
{
    int ret = read(fd, buf, sizeof(buf));
    if (ret < 0)
        close(fd);
}

// write
void OnConnection(int fd, char *buf, size_t len) 
{
    int ret = write(fd, buf, len);
    if (ret < 0)
        close(fd);
}


int main(int argc, char* argv[]) 
{
    std::string port = "10000";
    if (argc == 2) {
        port = argv[1];
    }
    std::string addr = std::string("127.0.0.1:") + port;
    EventLoop loop(0, 0);
    TCPClient client(&loop, addr, "TCPEcho");
    client.SetMessageCallback(&OnMessage);
    client.SetConnectionCallback(&OnConnection);
    client.Connect();
    loop.Run();
    return 0;
}


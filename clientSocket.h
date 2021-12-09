#ifndef CLIENT_SOCKET
#define CLIENT_SOCKET

#include<string>

#define BufferSize 10 * 1024 // 10kB

class ClientSocket
{
private:
    int _socket_desc = -1;
    char _buffer[BufferSize + 1];

public:
    ClientSocket(){}
    ClientSocket(int socket_desc);
    ClientSocket(std::string serverIp, std::string serverPort);
    ~ClientSocket();
    void Send(std::string message);
    std::string Recv();

};


#endif
#ifndef SERVER_SOCKET
#define SERVER_SOCKET

#include<string>

#include"clientSocket.h"

class ServerSocket
{
private:
    int _socket_desc = -1;
    int _accpeted_socket_desc = -1;
public:
    std::string Port;
    ServerSocket(){};
    ServerSocket(std::string port);
    ~ServerSocket();
    ClientSocket AcceptConnection();
};


#endif
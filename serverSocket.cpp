#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>

#include"serverSocket.h"
#include"ulti.h"

ServerSocket::ServerSocket(std::string portStr)
{
    // cache port
    Port = portStr;

	struct sockaddr_in server;

	// get int port
	int serverPort = -1;
	try
	{
		serverPort = std::stoi(portStr);
	}
	catch(const std::exception& e)
	{
		ExitProgram("invalid port number");
	}
	
	
	//Create socket
	_socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (_socket_desc == -1)
	{
		ExitProgram("Could not create socket");
	}
		
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(serverPort);

    // open server (bind)
    if( bind(_socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        ExitProgram("binding error, please make sure client port number is valid");
    }

    //Listen
    listen(_socket_desc , 3);
}

ClientSocket ServerSocket::AcceptConnection(){
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);
	_accpeted_socket_desc = accept(_socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (_accpeted_socket_desc<0)
	{
		ExitProgram("client to client socket accept failed");
	}

    return ClientSocket(_accpeted_socket_desc);
}

ServerSocket::~ServerSocket()
{

}


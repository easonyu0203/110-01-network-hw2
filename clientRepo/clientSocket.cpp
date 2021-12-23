#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>

#include"clientSocket.h"
#include"ulti.h"

ClientSocket::ClientSocket(std::string serverIp, std::string serverPortStr)
{
	// init buffer
	memset(_buffer, 0, BufferSize + 1);

	struct sockaddr_in server;

	// get int port
	int serverPort = -1;
	try
	{
		serverPort = std::stoi(serverPortStr);
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
		
	server.sin_addr.s_addr = inet_addr(serverIp.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(serverPort);

	//Connect to remote server
	if (connect(_socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		ExitProgram("connect error, please make sure server ip and port are correct");
	}

}

ClientSocket::ClientSocket(int socket_desc){
	_socket_desc = socket_desc;
	// init buffer
	memset(_buffer, 0, BufferSize + 1);
}

ClientSocket::~ClientSocket(){
	if(_socket_desc != -1){
		close(_socket_desc);
	}
}

void ClientSocket::Send(std::string message){
	if(send(_socket_desc, message.c_str() ,message.length() , 0) < 0)
	{
		ExitProgram("Send failed");
	}
}

std::string ClientSocket::Recv(){
	// clean buffer
	memset(_buffer, 0, BufferSize + 1);

	//recv and check error
	ssize_t bytes_readed = recv(_socket_desc, _buffer, BufferSize, 0);
	if(bytes_readed < 0){
		ExitProgram("socket recv data failed");
	}

	std::string msg = std::string(_buffer);
	if(msg == "Close\n"){
		ExitProgram("Server connection is full, disconnect to server");
	}

	return msg;
}
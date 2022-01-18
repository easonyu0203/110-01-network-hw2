#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>

#include"clientSocket.h"
#include"ulti.h"
#include"rsa.cpp"

ClientSocket::ClientSocket(std::string serverIp, std::string serverPortStr, bool f)
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

	if(f){
		// rsa socket
		auto[n,t,e,d] = encryption_key();
		this->n = n;
		this->t = t;
		this->e = e;
		this->d = d;

		// send public key & n
		string msg = "";
		msg = to_string(d) + "@" + to_string(n);
		cout << "ssl msg: "<< msg << "\n";
		send(_socket_desc, msg.c_str(), msg.length(), 0);

		// ger server key & n
		memset(_buffer, 0, BufferSize + 1);
		ssize_t bytes_readed = recv(_socket_desc, _buffer, BufferSize, 0);
		if(bytes_readed < 0){
			ExitProgram("socket recv data failed");
		}
		string res = std::string(_buffer);
		if(res == "Close\n"){
			ExitProgram("Server connection is full, disconnect to server");
		}
		this->serverD = stoi(res.substr(0, res.find('@')));
		this->serverN = stoi(res.substr(res.find('@')+1));
	}
	else{

	}

}

ClientSocket::ClientSocket(int socket_desc){
	_socket_desc = socket_desc;
	// init buffer
	memset(_buffer, 0, BufferSize + 1);

	// ssl set up
	int n, e, d;
	auto keys = encryption_key();
	this->n = std::get<0>(keys);
	this->e = std::get<2>(keys);
	this->d = std::get<3>(keys);
	// recv client ssl data
	ssize_t read_size = recv(socket_desc, _buffer, BufferSize, 0);
	std::string clientMessage = std::string(_buffer);
	this->serverD = std::stoi(clientMessage.substr(0, clientMessage.find('@')));
	this->serverN = std::stoi(clientMessage.substr(clientMessage.find('@')+1));
	// send server ssl data
	std::string msg = "";
	msg = std::to_string(this->d) + "@" + std::to_string(this->n);
	send(socket_desc, msg.c_str(), msg.length(), 0);
}

ClientSocket::~ClientSocket(){
	if(_socket_desc != -1){
		close(_socket_desc);
	}
}

void ClientSocket::Send(std::string message){
	message = encrypt(message, this->e, this->n);


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

	msg = decrypt(msg, this->serverD, this->serverN);

	return msg;
}
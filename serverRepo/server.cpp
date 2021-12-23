#include<iostream>
#include<string>
#include <stdexcept>
#include<sys/socket.h>
#include<netdb.h>	//hostent
#include<arpa/inet.h>
#include<unistd.h>	//write
#include <thread>  
#include <vector>
#include <chrono>
#include <iomanip>
#include<string.h>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

#include"ulti.h"

using namespace std::chrono;
using namespace std::this_thread;

#define BufferSize 1024 * 100
#define MaxClientCnt 3

std::string GetPortFromArgs(int argc, char const *argv[]);
void formalPrint(std::string title, std::string content);

struct UserInfo
{
    std::string Username;
    int AccountBalance;
    bool IsOnline;
    std::string Port;
    std::string IP;
    std::string S2CPort;

    UserInfo(std::string username = "[No set username]"){
        Username = username;
        AccountBalance = 10000;
        IsOnline = false;
        Port = "";
        IP = "";
    }
};


// data
std::string ServerPublicKey = "[##AA##]";
std::unordered_map<std::string, UserInfo> userMap;

std::string UserListStr(std::string username){
    std::string out = "";
    out += std::to_string(userMap[username].AccountBalance) + "\n";
    out += ServerPublicKey + "\n";
    
    // get all online user
    std::vector<UserInfo> onlineUserList;
    for(auto userInfo : userMap){
        if(userInfo.second.IsOnline == true){
            onlineUserList.push_back(userInfo.second);
        }
    }

    out += std::to_string(onlineUserList.size()) + "\n";
    for(UserInfo userInfo : onlineUserList){
        out += userInfo.Username + "#" + userInfo.IP + "#" + userInfo.Port + "\n";
    }
    return out;
}

// client request type mutex
std::mutex ClientHandlerMutex;

class ClientRequestType
{
private:
    /* data */
public:
    enum type{
        None,
        Register,
        Login,
        GetList,
        Exit,
        Transaction
    };
    static type GetType(std::string message){
        if(message.substr(0, 8) == "REGISTER") return Register;
        if(message == std::string("List")) return GetList;
        if(message == std::string("Exit")) return Exit;
        if(message.find_first_of('#') != message.find_last_of('#')) return Transaction;
        if(message.find("#") != std::string::npos) return Login;
        std::cout << "client give: [" << message << "] which server cant understand\n"; 
        return None;
    }

    static std::string HandleRegister(std::string message){
        // lock up
        std::lock_guard<std::mutex> guard(ClientHandlerMutex);

        // set return client message
        std::string returnMessage;

        // get userver name
        std::string username = message.substr(message.find('#')+1);
        if(username.find('\n') != std::string::npos) throw std::invalid_argument("username contain escape letter");

        // add to data
        // check have this user
        if(userMap.count(username) == 1){
            // already have this user
            returnMessage = "210 FAIL\n";
            formalPrint("Client Register Fail", "Already have this user");
        }
        else{
            userMap[username] = UserInfo(username);
            returnMessage = "100 OK\n";
            formalPrint("CLient Register Success", "username: " + username);
        }

        return returnMessage;
    }

    static std::string HandleLogin(std::string message, std::string clientIp, std::string clientPort){
        // lock up
        std::lock_guard<std::mutex> guard(ClientHandlerMutex);

        // set return client message
        std::string returnMessage = "220 AUTH_FAIL\n";

        // get username & port
        std::string username = message.substr(0, message.find('#'));
        std::string portStr = message.substr(message.find('#')+1);

        if(userMap.count(username) == 0){
            formalPrint("Login Fail", "username: " + username + "\nNot register yet");
            // dont have register
            returnMessage = "220 AUTH_FAIL\n";
        }
        else if(userMap[username].IsOnline == true){
            formalPrint("Login Fail", "username: " + username + "\nalready login");
            returnMessage = "220 AUTH_FAIL\n";
        }
        else{
            // have register
            formalPrint("Login Success", "username: " + username + "\nlogin to server");

            // log out previous on this ip user
            for(auto p : userMap){
                if(p.second.IP == clientIp && p.second.S2CPort == clientPort && p.second.IsOnline == true){
                    std::cout << "this ip, port have already login to other user\n";
                    userMap[p.second.Username].IsOnline = false;
                    break;
                }
            }

            // set data
            userMap[username].IsOnline = true;
            userMap[username].Port = portStr;
            userMap[username].IP = clientIp;
            userMap[username].S2CPort = clientPort;
            
            returnMessage = UserListStr(username);
        }
        return returnMessage;
    }

    static std::string HandleGetList(std::string ip, std::string clientPort){
        // lock up
        std::lock_guard<std::mutex> guard(ClientHandlerMutex);
        std::string returnMessage = "None";
        for(auto p : userMap){
            if(p.second.IP == ip && p.second.S2CPort == clientPort && p.second.IsOnline == true){
                returnMessage = UserListStr(p.second.Username);
                formalPrint("Client Request List", "username: " + p.second.Username);
            }
        }
        if(returnMessage == std::string("None")){
            formalPrint("Client Get List Fail", "Not login yet");
        }
        return returnMessage;
    }

    static std::string HandleExit(std::string ip, std::string clientPort){
        // lock up
        std::lock_guard<std::mutex> guard(ClientHandlerMutex);
        std::string returnMessage = "None";
        for(auto p : userMap){
            if(p.second.IP == ip && p.second.S2CPort == clientPort && p.second.IsOnline == true){
                userMap[p.second.Username].IsOnline = false;
                formalPrint("Client Exit", "username: " + p.second.Username);
            }
        }
        if(returnMessage == "None"){
            formalPrint("Client exit", "this client not login");
        }
        returnMessage = "Bye\n";
        return returnMessage;
    }

    static std::string HandleTransaction(std::string message, std::string ip, std::string clientPort){
        // lock up
        std::lock_guard<std::mutex> guard(ClientHandlerMutex);
        std::string returnMessage = "None";
        std::string senderUsername = "";
        for(auto p : userMap){
            if(p.second.IP == ip && p.second.S2CPort == clientPort && p.second.IsOnline == true){
                senderUsername = p.second.Username;
            }
        }
        if(senderUsername == std::string("")){
            formalPrint("Transaction Fail", "Client not login");
        }
        else{
            std::string aUser, bUser;
            int amount;
            size_t i1, i2;
            i1 = message.find('#');
            i2 = message.find('#', i1 + 1);
            aUser = message.substr(0, i1);
            bUser = message.substr(i2+1);
            amount = stoi(message.substr(i1+1, i2));

            if(senderUsername != bUser){
                formalPrint("Transaction Fail", "wrong client send \n transaction request");
            }
            else{
                formalPrint("Transaction Success", (
                   "Sender  : " + aUser + "\n" +\
                   "Receiver: " + bUser + "\n" +\
                   "Amount  : " + std::to_string(amount) + "\n" 
                ));
                userMap[aUser].AccountBalance -= amount;
                userMap[bUser].AccountBalance += amount;
                returnMessage = "100\n";
            }

        }

        return returnMessage;
    }
};


int finishCnt = 0;
//Accept and incoming connection
std::vector<std::thread> ClientHandlerThreads;
// arg
// port
int main(int argc, char const *argv[])
{
    // get port from arg
    std::string portStr = GetPortFromArgs(argc, argv);
        
    //setting up server
	int socket_desc;
	struct sockaddr_in server;
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) ExitProgram("Could not create socket");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( std::stoi(portStr) );
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) ExitProgram("bind failed");
	listen(socket_desc , 3);
	puts("Waiting for incoming connections...");
    while (true)
    {
        std::cout << "Client Cnt: " << ClientHandlerThreads.size() - finishCnt << "\n";
        // accep a new client
        int new_socket, c;
        struct sockaddr_in client;
        c = sizeof(struct sockaddr_in);
        new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);

        if(ClientHandlerThreads.size() - finishCnt >= MaxClientCnt){
            formalPrint("Too Many Client", "can only have 3 clients");
            std::string closeMsg = "Close\n";
            write(new_socket, closeMsg.c_str(), closeMsg.size());
            close(new_socket);
        }
        else{
            // client connected, handle communication
            ClientHandlerThreads.emplace_back(
                std::thread(
                    [=](){
                        if (new_socket<0) ExitProgram("accept failed");

                        std::string client_ip = inet_ntoa(client.sin_addr);
                        int client_port = ntohs(client.sin_port);

                        formalPrint("New Client Connect Event",(
                            "IP  : " + client_ip + "\n"\
                            "Port: " + std::to_string(client_port)
                        ));

                        // open a buffer for recv client data
                        char buffer[BufferSize + 1];

                        // waiting client send request
                        while(true){
                            memset(buffer, 0, BufferSize + 1);
                            ssize_t read_size = recv(new_socket, buffer, BufferSize, 0);

                            // handle disconnect
                            if(read_size == 0)
                            {
                                formalPrint("Client disconnected", (
                                    "IP  : " + client_ip + "\n"\
                                    "Port: " + std::to_string(client_port)
                                ));
                                finishCnt++;
                                std::cout << "Client Cnt: " << ClientHandlerThreads.size() - finishCnt << "\n";
                                return;
                            }
                            // handle recv fail
                            else if(read_size == -1)
                            {
                                ExitProgram("recv failed");
                            }

                            std::string clientMessage = std::string(buffer);
                            std::string returnMessage;

                            // handle different client message type
                            switch (ClientRequestType::GetType(clientMessage))
                            {
                            case ClientRequestType::None:
                                formalPrint("Client Message Invalid",(
                                    "IP  : " + client_ip + "\n"\
                                    "Port: " + std::to_string(client_port)       
                                ));
                                break;
                            case ClientRequestType::Register:
                                returnMessage = ClientRequestType::HandleRegister(clientMessage);
                                write(new_socket, returnMessage.c_str(), returnMessage.size());
                                break;
                            case ClientRequestType::Login:
                                returnMessage = ClientRequestType::HandleLogin(clientMessage, client_ip, std::to_string(client_port));
                                write(new_socket, returnMessage.c_str(), returnMessage.size());
                                break;
                            case ClientRequestType::GetList:
                                returnMessage = ClientRequestType::HandleGetList(client_ip, std::to_string(client_port));
                                write(new_socket, returnMessage.c_str(), returnMessage.size());
                                break;
                            case ClientRequestType::Exit:
                                returnMessage = ClientRequestType::HandleExit(client_ip, std::to_string(client_port));
                                write(new_socket, returnMessage.c_str(), returnMessage.size());
                                break;
                            case ClientRequestType::Transaction:
                                returnMessage = ClientRequestType::HandleTransaction(clientMessage, client_ip, std::to_string(client_port));
                                break;
                            default:
                                formalPrint("Wrong Formate", "Client send wrong message");
                                break;
                            }
                        }
                    }
                )
            );

        }
    }

    for(auto& t : ClientHandlerThreads) {t.join();}

    return 0;
}

std::mutex FormalPrintMutex;
void formalPrint(std::string title, std::string content){
    // lock up 
    std::lock_guard<std::mutex> guard(FormalPrintMutex);

    std::string sep = "||=========================||\n"; // 22
    std::string innerSep = "||-------------------------||\n";

    std::cout << "\n" << sep;
    std::cout << "||" << std::setw(25) << std::left << title << "||\n";
    std::cout << innerSep;
    auto lines = Split(content, "\n");
    for(std::string line : lines){
        std::cout << "||" << std::setw(25) << std::left << line << "||\n";
    }
    std::cout << sep;
}



std::string GetPortFromArgs(int argc, char const *argv[]){
    if(argc < 2) throw std::invalid_argument("arg dont have port");
    std::string portStr = argv[1]; 
    return portStr;
}

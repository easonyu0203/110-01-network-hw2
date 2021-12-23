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

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

#define BufferSize 1024 * 100

std::string GetPortFromArgs(int argc, char const *argv[]);
void formalPrint(string title, string content);

struct UserInfo
{
    string Username;
    int AccountBalance;
    bool IsOnline;
    string Port;
    string IP;
    string S2CPort;

    UserInfo(string username = "[No set username]"){
        Username = username;
        AccountBalance = 10000;
        IsOnline = false;
        Port = "";
        IP = "";
    }
};


// data
string ServerPublicKey = "[##AA##]";
unordered_map<string, UserInfo> userMap;

string UserListStr(string username){
    string out = "";
    out += to_string(userMap[username].AccountBalance) + "\n";
    out += ServerPublicKey + "\n";
    
    // get all online user
    vector<UserInfo> onlineUserList;
    for(auto userInfo : userMap){
        if(userInfo.second.IsOnline == true){
            onlineUserList.push_back(userInfo.second);
        }
    }

    out += to_string(onlineUserList.size()) + "\n";
    for(UserInfo userInfo : onlineUserList){
        out += userInfo.Username + "#" + userInfo.IP + "#" + userInfo.Port + "\n";
    }
    return out;
}

// client request type mutex
mutex ClientHandlerMutex;

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
    static type GetType(string message){
        if(message.substr(0, 8) == "REGISTER") return Register;
        if(message == string("List")) return GetList;
        if(message == string("Exit")) return Exit;
        if(message.find_first_of('#') != message.find_last_of('#')) return Transaction;
        if(message.find("#") != string::npos) return Login;
        cout << "client give: [" << message << "] which server cant understand\n"; 
        return None;
    }

    static string HandleRegister(string message){
        // lock up
        lock_guard<mutex> guard(ClientHandlerMutex);

        // set return client message
        string returnMessage;

        // get userver name
        string username = message.substr(message.find('#')+1);
        if(username.find('\n') != string::npos) throw invalid_argument("username contain escape letter");

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

    static string HandleLogin(string message, string clientIp, string clientPort){
        // lock up
        lock_guard<mutex> guard(ClientHandlerMutex);

        // set return client message
        string returnMessage = "220 AUTH_FAIL\n";

        // get username & port
        string username = message.substr(0, message.find('#'));
        string portStr = message.substr(message.find('#')+1);

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
                    cout << "this ip, port have already login to other user\n";
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

    static string HandleGetList(string ip, string clientPort){
        // lock up
        lock_guard<mutex> guard(ClientHandlerMutex);
        string returnMessage = "None";
        for(auto p : userMap){
            if(p.second.IP == ip && p.second.S2CPort == clientPort && p.second.IsOnline == true){
                returnMessage = UserListStr(p.second.Username);
                formalPrint("Client Request List", "username: " + p.second.Username);
            }
        }
        if(returnMessage == string("None")){
            formalPrint("Client Get List Fail", "Not login yet");
        }
        return returnMessage;
    }

    static string HandleExit(string ip, string clientPort){
        // lock up
        lock_guard<mutex> guard(ClientHandlerMutex);
        string returnMessage = "None";
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

    static string HandleTransaction(string message, string ip, string clientPort){
        // lock up
        lock_guard<mutex> guard(ClientHandlerMutex);
        string returnMessage = "None";
        string senderUsername = "";
        for(auto p : userMap){
            if(p.second.IP == ip && p.second.S2CPort == clientPort && p.second.IsOnline == true){
                senderUsername = p.second.Username;
            }
        }
        if(senderUsername == string("")){
            formalPrint("Transaction Fail", "Client not login");
        }
        else{
            string aUser, bUser;
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
                   "Amount  : " + to_string(amount) + "\n" 
                ));
                userMap[aUser].AccountBalance -= amount;
                userMap[bUser].AccountBalance += amount;
                returnMessage = "100\n";
            }

        }

        return returnMessage;
    }
};


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


	//Accept and incoming connection
    std::vector<std::thread> ClientHandlerThreads;
    while (true)
    {
        // accep a new client
        int new_socket, c;
        struct sockaddr_in client;
        c = sizeof(struct sockaddr_in);
        new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);

        // client connected, handle communication
        ClientHandlerThreads.emplace_back(
            std::thread(
                [=](){
                    if (new_socket<0) ExitProgram("accept failed");

                    std::string client_ip = inet_ntoa(client.sin_addr);
                    int client_port = ntohs(client.sin_port);

                    formalPrint("New Client Connect Event",(
                        "IP  : " + client_ip + "\n"\
                        "Port: " + to_string(client_port)
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
                                "Port: " + to_string(client_port)
                            ));
                            return;
                        }
                        // handle recv fail
                        else if(read_size == -1)
                        {
                            ExitProgram("recv failed");
                        }

                        string clientMessage = string(buffer);
                        string returnMessage;

                        // handle different client message type
                        switch (ClientRequestType::GetType(clientMessage))
                        {
                        case ClientRequestType::None:
                            formalPrint("Client Message Invalid",(
                                "IP  : " + client_ip + "\n"\
                                "Port: " + to_string(client_port)       
                            ));
                            break;
                        case ClientRequestType::Register:
                            returnMessage = ClientRequestType::HandleRegister(clientMessage);
                            write(new_socket, returnMessage.c_str(), returnMessage.size());
                            break;
                        case ClientRequestType::Login:
                            returnMessage = ClientRequestType::HandleLogin(clientMessage, client_ip, to_string(client_port));
                            write(new_socket, returnMessage.c_str(), returnMessage.size());
                            break;
                        case ClientRequestType::GetList:
                            returnMessage = ClientRequestType::HandleGetList(client_ip, to_string(client_port));
                            write(new_socket, returnMessage.c_str(), returnMessage.size());
                            break;
                        case ClientRequestType::Exit:
                            returnMessage = ClientRequestType::HandleExit(client_ip, to_string(client_port));
                            write(new_socket, returnMessage.c_str(), returnMessage.size());
                            break;
                        case ClientRequestType::Transaction:
                            returnMessage = ClientRequestType::HandleTransaction(clientMessage, client_ip, to_string(client_port));
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

    for(auto& t : ClientHandlerThreads) {t.join();}

    return 0;
}

std::mutex FormalPrintMutex;
void formalPrint(string title, string content){
    // lock up 
    std::lock_guard<mutex> guard(FormalPrintMutex);

    string sep = "||=========================||\n"; // 22
    string innerSep = "||-------------------------||\n";

    cout << "\n" << sep;
    cout << "||" << setw(25) << left << title << "||\n";
    cout << innerSep;
    auto lines = Split(content, "\n");
    for(string line : lines){
        cout << "||" << setw(25) << left << line << "||\n";
    }
    cout << sep;
}



std::string GetPortFromArgs(int argc, char const *argv[]){
    if(argc < 2) throw std::invalid_argument("arg dont have port");
    std::string portStr = argv[1]; 
    return portStr;
}

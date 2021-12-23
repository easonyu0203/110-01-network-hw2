#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<thread>
#include<stdexcept>
#include<utility> // std::pair
#include <iomanip>
#include<unistd.h>	//write

#include"clientSocket.h"
#include"serverSocket.h"
#include"ulti.h"

#define SepLine "==========================="



void HandleRegisterAction(ClientSocket&, ServerSocket&);
void HandleLoginAction(ClientSocket&, ServerSocket&);
void HandleGetListAction(ClientSocket&, ServerSocket&);
void HandleExitAction(ClientSocket&, ServerSocket&);
void HandleTransactionAction(ClientSocket&, ServerSocket&);
void List(std::vector<std::string> df);
void UpdateUserInfoDict(ClientSocket &ToServerSocket, ServerSocket &LocalServerSocket);

// client action enum
enum ActionEnum{
    Register,
    Login,
    GetList,
    Exit,
    Transaction,
};

struct UserInfo
{
    std::string UserName;
    std::string UserIp;
    std::string UserPort;
    UserInfo(){}
    UserInfo(std::string userName, std::string ip, std::string port){
        UserName = userName;
        UserIp = ip;
        UserPort = port;
    }
};

struct UserCredentials{
    std::string accountBalance;
};

std::unordered_map<std::string, UserInfo> UserInfodict;
std::string LocalUserName;


std::string GetActionStr(ActionEnum action){
    std::string out = "";
    switch (action)
    {
    case Register:
        out = "Register";
        break;
    case Login:
        out = "Login";
        break;
    case GetList:
        out = "GetList";
        break;
    case Exit:
        out = "Exit";
        break;
    case Transaction:
        out = "Transaction";
        break;
    default:
        throw std::invalid_argument("no match for received action enum");
        break;
    }

    return out;
}

std::vector<ActionEnum> ActionList({
    Register,
    Login,
    GetList,
    Exit,
    Transaction,
});

std::unordered_map<int, ActionEnum> DisplayChoosingText(){
    // make display string and int->action dictionary
    std::string displayStr = "";
    std::unordered_map<int, ActionEnum> actionDict;
    displayStr += "Pick an action:\n";
    for(int i = 0; i < ActionList.size(); i++){
        // add choice to display
        displayStr += std::string("  [") + std::to_string(i+1) + "] " + GetActionStr(ActionList[i]) + "\n";
        // ad choice to dict
        actionDict[i+1] = ActionList[i];

    }

    // display text and get user input action
    // display choices
    std::cout << displayStr;

    return actionDict;
}

// get user input action
ActionEnum ChooseAction(){
    //Dispaly choosing text
    auto actionDict = DisplayChoosingText();

    std::cout << "Please enter your numeric choice:  ";


    // get user valid choice
    std::string choiceStr;
    int choiceInt;
    while(true){
        // get user choice
        std::cin >> choiceStr;

        // check choice valid
        try
        {
            choiceInt = std::stoi(choiceStr);
            if(choiceInt > 0 && choiceInt < 6){
                // success input
                break;
            }
            else{
                std::cout << "Please enter a value between 1 and " << ActionList.size() << ":   ";
            }
        }
        catch(const std::exception& e)
        {
            std::cout << "Please enter a value between 1 and " << ActionList.size() << ":   ";
        }

    }

    return actionDict[choiceInt];
}

std::string GetLocalServerPortByArg(int argc, char const *argv[]){
    // set up arg vector
    std::vector<std::string> argVec;
    for(int i = 0; i < argc; i++){
        argVec.push_back(std::string(argv[i]));
    }

    //default port
    std::string localServerPort = "7777";

    if(argVec.size() < 4) return localServerPort;
    localServerPort = argVec[3];
    return localServerPort;
}

std::pair<std::string, std::string> GetServerAddressByArg(int argc, char const *argv[]){
    // set up arg vector
    std::vector<std::string> argVec;
    for(int i = 0; i < argc; i++){
        argVec.push_back(std::string(argv[i]));
    }

    // default ip, port
    std::string serverIp = "127.0.0.1";
    std::string serverPort = "8888";

    // return default if not enough arg input
    if(argVec.size() < 3) return std::make_pair(serverIp, serverPort);
    
    serverIp = argVec[1];
    serverPort = argVec[2];
    return std::make_pair(serverIp, serverPort);
}

void ActionHandling(ActionEnum action, ClientSocket &ToServerSocket, ServerSocket &LocalServerSocket){
    // Top divider
    std::cout << "\n";
    std::cout << SepLine << "\n";

    switch (action)
    {
    case Register:
        HandleRegisterAction(ToServerSocket, LocalServerSocket);
        break;
    case Login:
        HandleLoginAction(ToServerSocket, LocalServerSocket);
        break;
    case GetList:
        HandleGetListAction(ToServerSocket, LocalServerSocket);
        break;
    case Exit:
        HandleExitAction(ToServerSocket, LocalServerSocket);
        break;
    case Transaction:
        HandleTransactionAction(ToServerSocket, LocalServerSocket);
        break;
    default:
        throw std::invalid_argument("no match for received action enum");
        break;
    }
    
    // bottom divider
    std::cout << SepLine << "\n\n";
}

void HandleRegisterAction(ClientSocket &ToServerSocket, ServerSocket &LocalServerSocket){
    std::string username = "";

    // input username
    std::cout << "REGISTER\n";
    std::cout << "Please enter a user name:  ";
    std::cin >> username;

    // send register request to server
    std::string sendMessage = "REGISTER#" + username;
    ToServerSocket.Send(sendMessage);


    // revc from server
    std::string recvMessage;
    recvMessage = ToServerSocket.Recv();

    if(recvMessage.substr(0, 3) == "100"){
        // register success
        std::cout << "Register successfully\n";
    }
    else{
        // register fail
        std::cout << "Register failed\n";
    }
}
void HandleLoginAction(ClientSocket &ToServerSocket, ServerSocket &LocalServerSocket){
    std::string username = "";

    // input username
    std::cout << "LOGIN\n";
    std::cout << "Please enter a user name:  ";
    std::cin >> username;

    // set local user name
    LocalUserName = username;

    // send login request to server
    std::string sendMessage = username + "#" + LocalServerSocket.Port;
    ToServerSocket.Send(sendMessage);

    // revc from server
    std::string recvMessage = ToServerSocket.Recv();
    auto recvList = Split(recvMessage, "\n");

    if(recvList.size() <= 3){
        std::cout << "Login Fail\n";
        std::cout << "[No user online]\n";
        return;
    }
    else{
        std::cout << "Login Success\n";
    }

    //get list df
    auto first = recvList.begin() + 3;
    auto last = recvList.end();

    List(std::vector<std::string>(first, last));
}

void HandleGetListAction(ClientSocket &ToServerSocket, ServerSocket &LocalServerSocket){
    std::cout << "LIST\n";

    // send to server
    ToServerSocket.Send("List");

    // revc from server
    std::string recvMessage = ToServerSocket.Recv();
    auto recvList = Split(recvMessage, "\n");
    
    std::cout << "Account Balance: " << recvList[0] << "\n";

    if(recvList.size() <= 3){
        std::cout << "[No user online]\n";
        return;
    }

    //get list df
    auto first = recvList.begin() + 3;
    auto last = recvList.end();
    List(std::vector<std::string>(first, last));
}

void HandleExitAction(ClientSocket &ToServerSocket, ServerSocket &LocalServerSocket){
    std::cout << "EXIT\n";

    ToServerSocket.Send("Exit");
    std::string recvMessage = ToServerSocket.Recv();

    // exit
    exit(0);
}

void HandleTransactionAction(ClientSocket &ToServerSocket, ServerSocket &LocalServerSocket){
    // update user info dict
    UpdateUserInfoDict(ToServerSocket, LocalServerSocket);

    std::string username, amountStr;
    int amount;
    std::cout << "TRANSACTION\n";
    std::cout << "Please enter the user name to transact:  ";
    std::cin >> username;

    // check have user
    if(UserInfodict.count(username) == 0){
        std::cout << "can't find user: " + username << "\n";
        return;
    }

    std::cout << "Please enter the pay amount to transact:  ";
    std::cin >> amountStr;

    // check amount valid
    try
    {
        amount = std::stoi(amountStr);
    }
    catch(const std::exception& e)
    {
        std::cout << "pay amount invalid\n";
        return;
    }

    std::cout << "Transaction processing...\n";

    // connect to peer
    UserInfo peerInfo = UserInfodict[username];
    ClientSocket ToPeerSocket = ClientSocket(peerInfo.UserIp, peerInfo.UserPort);

    //send message to peer
    std::string msg =  LocalUserName + "#" + amountStr + "#" + peerInfo.UserName;
    ToPeerSocket.Send(msg);

    // receive confirm from server
    std::cout << "Transaction success!\n";
}


void UpdateUserInfoDict(ClientSocket &ToServerSocket, ServerSocket &LocalServerSocket){
    UserInfodict.clear();

    // send to server
    ToServerSocket.Send("List");

    // revc from server
    std::string recvMessage = ToServerSocket.Recv();
    auto recvList = Split(recvMessage, "\n");

    if(recvList.size() <= 3){
        return;
    }

    //get list df
    auto first = recvList.begin() + 3;
    auto last = recvList.end();

    for(auto it = first; it != last; it++){
        auto tokens = Split(*it, "#");
        if(tokens.size() < 3) continue;
        UserInfodict[tokens[0]] = (UserInfo(tokens[0],tokens[1],tokens[2]));
    }

}

void List(std::vector<std::string> df){
    std::string l10 = "----------";
    std::cout << "|" << l10 << "|" << l10 << l10 << "|" << l10 << "|\n";
    std::cout << "|" << std::setw(10) << "Username" << "|" << std::setw(20) << "IP" << "|" << std::setw(10) << "Port" << "|" << "\n"; 
    for(auto line: df){
        auto tokens = Split(line, "#");
        if(tokens.size() < 3) continue;
        std::cout << "|" << std::setw(10) << tokens[0] << "|" << std::setw(20) << tokens[1] << "|" << std::setw(10) << tokens[2] << "|" << "\n"; 
    }
    std::cout << "|" << l10 << "|" << l10 << l10 << "|" << l10 << "|\n";

}

// arg: serverIp serverPort localServerPort
int main(int argc, char const *argv[])
{
    // get server ip, port & local server port
    auto[serverIp, serverPort] = GetServerAddressByArg(argc, argv);
    std::string localServerPort = GetLocalServerPortByArg(argc, argv);

    // connect to server
    ClientSocket ToServerSocket = ClientSocket(serverIp, serverPort);

    //open local server
    ServerSocket LocalServerSocket = ServerSocket(localServerPort);
    std::thread listenPeerJob([&](){
        while(true){
            ClientSocket peerSocket = LocalServerSocket.AcceptConnection();
            //get msg from peer
            auto msg = peerSocket.Recv();
            auto tokens = Split(msg, "#");
            std::cout << "\n\n[You Receive a transaction]\n";
            std::cout << "||==========||==========||\n";
            std::cout << "|| User Name||Pay Amount||\n";
            std::cout << "||" << std::setw(10) << tokens[0] << "||" << std::setw(10) << tokens[1] << "||\n";
            std::cout << "||==========||==========||\n";

            //give to server
            ToServerSocket.Send(msg);

            DisplayChoosingText();
            std::cout << "Please enter your numeric choice:  ";

        }
    });

    while (true)
    {
        // get user action
        ActionEnum inputAction = ChooseAction();

        // handle action
        ActionHandling(inputAction, ToServerSocket, LocalServerSocket);

    }

    listenPeerJob.join();

    return 0;
}
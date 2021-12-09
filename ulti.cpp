#include"ulti.h"

#include<string>
#include<vector>

void ExitProgram(std::string why){
    std::string sep = "===========================";

    std::cout << "\n" << sep << "\n";
    std::cout << "Program kill\n";
    std::cout << "Reason: " << why << std::endl;
    std::cout << sep << "\n";

    exit(1);
}

std::vector<std::string> Split(std::string str, std::string delimiter){
    std::vector<std::string> out;

    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        out.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    out.push_back(str);
    return out;
}
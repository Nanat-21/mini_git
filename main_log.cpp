#include "mini_git.hpp"
#include <iostream>

int main() {
    MiniGitRepo repo; 

    std::string command;
    std::cout << "Enter command: ";
    std::cin >> command;

    if (command == "init") {
        repo.init();
    }
    else if ( command=="add"){
        std::string filename;
        std::cin>>filename;
        repo.add(filename);
    }
    else if(command=="commit"){
        std::string flag;
        std::cin>> flag;
        if(flag == "-m"){
            std::string message;
            std::getline(std::cin>>std::ws, message);
            repo.commit(message);
        }else{
            std::cout<<"usage: commit -m\"message\"\n";
        }
    }
    else if(command == "log"){
        repo.log();
    }
    else{
        std::cout << "Unknown command\n";
    }

    return 0;
}
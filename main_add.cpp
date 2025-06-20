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
    else{
        std::cout << "Unknown command\n";
    }

    return 0;
}





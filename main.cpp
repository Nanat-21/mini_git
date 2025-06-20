#include "mini_git.hpp"
#include <iostream>

int main() {
    MiniGitRepo repo;

    std::string command;
    std::cout << "Enter command: ";
    std::cin >> command;

    if (command == "init") {
        repo.init();
    } else {
        std::cout << "Unknown command\n";
    }

    return 0;
}
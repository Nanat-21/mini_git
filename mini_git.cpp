#include "mini_git.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void MiniGitRepo::init(){
    if (fs::exists(baseDir)){
        std::cout<<"MiniGit is already initialized.\n";
        return;
    }

    //create .mingit structure
    fs::create_directory(baseDir);
    fs::create_directory(objectsDir);
    fs::create_directory(refsDir);

    //create Head file pointing to main branch
    std::ofstream head(headFile);
    head<<"ref: refs/main\n";
    head.close();

    //create empty main branch ref file
    std::ofstream mainRef(mainRefFile);
    mainRef << "null";
    mainRef.close();

    std::cout<< "Initialized empty MiniGit repository in" << fs::absolute(baseDir) <<"\n";
}

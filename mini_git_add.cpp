#include "mini_git.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include<sstream>
#include<functional>

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

std::string computeHash(const std::string& content){
    std::hash<std::string> hasher;
    size_t hash = hasher( content);
    std::stringstream ss;
    ss<< std::hex << hash;
    return ss.str();
}

void MiniGitRepo::add(const std::string& filename){
    if(!fs::exists(filename)){
        std::cout<<"Error: File '"<< filename<<"'does not exist.\n";
        return;
    }

    //Read file content
    std::ifstream inFile(filename);
    std::stringstream buffer;
    buffer<< inFile.rdbuf();
    std::string content = buffer.str();

    //compute hash
    std::string hash = computeHash(content);

    //store blob only if not already stored
    std::string blobpath = objectsDir + "/" + hash;
    if(!fs::exists(blobpath)){
        std::ofstream out(blobpath);
        out.close();
        std::cout<<"Add blob: "<<hash<< "\n";
    } else {
        std::cout<<"Blob already exists. Skipping file copy.\n";
    }
    std::cout<<" Staged file: "<< filename<<"\n";
}

#include "mini_git.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include<sstream>
#include<functional>
#include<ctime>

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

    std::string hash= computeHash(content);
    std::string blobPath = objectsDir + "/" + hash;

    //store blob if it doesn`t exist 
    if (!fs::exists(blobPath)){
        std::ofstream out(blobPath);
        out<<content;
        out.close();
        std::cout<<"Added blob: "<< hash <<"\n";
    }else{
        std::cout<<"Blob alraedy exists. Skipping file copy.\n";
    }

    //Append filename to index if not already staged
    std::ifstream indexcheck(baseDir + "/index");
    std::string line;
    bool alreadyStaged = false;
    while (std::getline(indexcheck,line)){
        if(line == filename){
            alreadyStaged = true;
            break;
        }
    }
    indexcheck.close();

    if(!alreadyStaged){
        std::ofstream indexOut(baseDir + "/index", std::ios::app);
        indexOut<< filename<<"\n";
        indexOut.close();
    }
    std::cout<<"Staged file: "<< filename<<"\n";
}

void MiniGitRepo::commit(const std::string& message){
    //HEAD -> refs/main -> current commit hash
    std::ifstream headIn(headFile);
    std::string headRef;
    std::getline(headIn,headRef);
    headIn.close();

    std::string branch = headRef.substr(5);
    std::string branchPath= baseDir + "/" + branch;

    std::ifstream branchIn(branchPath);
    std::string parentHash;
    std::getline(branchIn, parentHash); 
    branchIn.close();

    time_t now = time(0);
    std::string timestamp = ctime(&now);
    std::string combined = message + timestamp;
    std::string commitHash = computeHash(combined);

    //create commit file
    fs::create_directory(baseDir + "/commits");
    std::string commitPath = baseDir + "/commits/" + commitHash + ".txt";

    std::ofstream commitFile(commitPath);
    commitFile<< "message: "<<message<<"\n";
    commitFile<<"timestamp: "<<timestamp;
    commitFile<<"parent: "<<(parentHash == "null"? "null":parentHash)<<"\n";
    commitFile<<"files:\n";

    //Read staged file from index
    std::ifstream indexIn(baseDir + "/index");
    std::string filename;
    while(std::getline(indexIn, filename)){
        std::ifstream fileIn(filename);
        std::stringstream buffer;
        buffer<< fileIn.rdbuf();
        std::string content= buffer.str();
        std::string hash = computeHash(content);
        commitFile<<" "<<filename<<" -> "<< hash <<"\n";
    }
    indexIn.close();
    commitFile.close();

    //update branch pointer
    std::ofstream branchOut(branchPath);
    branchOut << commitHash;
    branchOut.close();

    //clear the index after commit
    std::ofstream clearIndex(baseDir + "/index", std::ios::trunc);
    if(clearIndex.is_open()){
        clearIndex.close();
        std::cout<<"Index cleared after commit.\n";
    }else{
        std::cout<<"warning: could not clear index.\n";
    }
}

void MiniGitRepo::log() {
    // Step 1: Read HEAD to get current branch
    std::ifstream headIn(headFile);
    std::string headRef;
    std::getline(headIn, headRef);  // e.g., "ref: refs/main"
    headIn.close();

    std::string branch = headRef.substr(5);  // remove "ref: "
    std::string branchPath = baseDir + "/" + branch;

    // Step 2: Get latest commit hash
    std::ifstream branchIn(branchPath);
    std::string currentHash;
    std::getline(branchIn, currentHash);
    branchIn.close();

    if (currentHash == "null" || currentHash.empty()) {
        std::cout << "No commits yet.\n"; 
        return;
    }
// Step 3: Follow the parent chain and print commits
    while (currentHash != "null") {
        std::string commitPath = baseDir + "/commits/" + currentHash + ".txt";
        std::ifstream commitIn(commitPath);
        if (!commitIn) {
            std::cout << "Error: Commit file not found: " << commitPath << "\n";
            break;
        }
        std::string line;
        std::string message, timestamp, parent;

        while (std::getline(commitIn, line)) {
            if (line.rfind("message:", 0) == 0)
                message = line.substr(8);
            else if (line.rfind("timestamp:", 0) == 0)
                timestamp = line.substr(10);
            else if (line.rfind("parent:", 0) == 0)
                parent = line.substr(7);
                parent.erase(0, parent.find_first_not_of(" \t\r\n"));
                parent.erase(parent.find_last_not_of(" \t\r\n")+1);
        }

        std::cout << "-------------------------------\n";
        std::cout << "Commit Hash: " << currentHash << "\n";
        std::cout << "Message:     " << message << "\n";
        std::cout << "Timestamp:   " << timestamp;
        std::cout << "-------------------------------\n";

        currentHash = parent;
    }
}
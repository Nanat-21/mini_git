#include "mini_git.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include<sstream>
#include<functional>
#include<ctime>
#include<map>
#include<string>
#include<set>
#include<iomanip>
#include <algorithm>

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
        if (!fs::exists(filename)) {
            std::cout << "Warning: File '" << filename << "' was staged but no longer exists. Skipping.\n";
            continue;
        }

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

void MiniGitRepo::branch(const std::string& branchName){
    //Get current branch from HEAD
    std::ifstream headIn(headFile);
    std::string headRef;
    std::getline(headIn,headRef);
    headIn.close();

    std::string currentBranchPath= baseDir + "/" + headRef.substr(5);  //remove "ref: "
    std::ifstream currentBranchIn(currentBranchPath);
    std::string currentCommitHash;
    std::getline(currentBranchIn, currentCommitHash);
    currentBranchIn.close();

    std::string newBranchPath = refsDir + "/" + branchName;

    //check if branch already exists
    if(fs::exists(newBranchPath)){
        std::cout<<" Error: Branch '"<< branchName<< "'already exists.\n";
        return;
    }

    //create the new branch file and point it to current commit
    std::ofstream newBranchOut(newBranchPath);
    newBranchOut<< currentCommitHash;
    newBranchOut.close();

    std::cout<<"Branch '"<<branchName<<"'created at commit"<< currentCommitHash<<"\n";
}
void MiniGitRepo::checkout(const std::string& branchName){
    std::string newBranchPath = refsDir + "/" + branchName;

    if(!fs::exists(newBranchPath)){
        std::cout<<"Error: Branch '"<<branchName<<"' does not exist.\n";
        return;
    }

    //update HEAD to point to the new branch
    std::ofstream headOut(headFile);
    headOut<< "ref: refs/"<< branchName<<"\n";
    headOut.close();

    std::cout<<"Switched to branch '"<< branchName<<"'\n";

    //Read the latest commit hash from the branch
    std::ifstream branchIn(newBranchPath);
    std::string latestCommitHash;
    std::getline(branchIn, latestCommitHash);
    branchIn.close();

    if(latestCommitHash == "null"|| latestCommitHash.empty()){
        std::cout<<"No commits on this branch yet.\n";
        return;
    }

    //open the commit file and parse file mappings
    std::string commitFilePath = baseDir + "/commits/" + latestCommitHash + ".txt";
    if(!fs::exists(commitFilePath)){
        std::cout<<"No commits yet on branch'"<<branchName<<"'\n";
        return;
    }
    std::ifstream commitFile(commitFilePath);

    std::string line;
    bool fileSection = false;

    while(std::getline(commitFile,line)){
        if(line== "files:"){
            fileSection= true;
            continue;
        }
        if(fileSection && !line.empty()){
            size_t arrow = line.find("->");
            if(arrow == std::string::npos) continue;

            std::string filename= line.substr(0, arrow-1);
            std::string hash = line.substr(arrow+3);

            //trim spaces
            filename.erase(0, filename.find_first_not_of(" \t\r\n"));
            filename.erase(filename.find_last_not_of(" \t\r\n")+1);
            hash.erase(0, hash.find_first_not_of(" \t\r\n"));
            hash.erase(hash.find_last_not_of(" \t\r\n") + 1);

            std::ifstream blob(objectsDir + "/" + hash );
            if(!blob.is_open()){
                std::cout<<"Error: blob not found for file: "<< filename<<"\n";
                continue;
            }
            std::ofstream outFile(filename);
            outFile << blob.rdbuf();
            blob.close();
            outFile.close();

            std::cout<<"Restored: "<<filename<<'\n';
        }
    }

    commitFile.close();
}

std::string findLCA(const std::string& commit1, const std::string& commit2){
    std::set<std::string> ancestors;

    //Traverse commit1 ancestors
    std::string current = commit1;
    while( current != "null" && !current.empty()){
        ancestors.insert(current);
        std::ifstream in(".minigit/commits/" + current + ".txt");
        std::string line;
        while(std::getline(in, line)){
            if(line.rfind("parent:", 0) == 0){
                current = line.substr(7);
                current.erase(0, current.find_first_not_of(" \t\r\n"));
                current.erase(current.find_last_not_of(" \t\r\n") + 1);
                break;
            }
        }
        in.close();
    }
    //Traverse commit2 ancestors to find first common
    current = commit2;
    while(current != "null" && !current.empty()){
        if(ancestors.count(current)) return current;
        std::ifstream in(".minigit/commits/" + current + ".txt");
        std::string line;
        while(std::getline(in, line)){
            if(line.rfind("parent:", 0)== 0){
                current= line.substr(7);
                current.erase(0, current.find_first_not_of(" \t\r\n"));
                current.erase(current.find_last_not_of("\t\r\n") + 1);
                break;
            }
        }
        in.close();
    }

    return "null";  //No common ancestor found
}

void MiniGitRepo::merge(const std::string& otherBranchName){
    //Load current branch from HEAD
        std::ifstream headIn(headFile);
        std::string headRef;
        std::getline(headIn, headRef);
        headIn.close();

    std::string currentBranch = headRef.substr(5); //remove "ref: "
    std::string currentBranchPath = baseDir + "/"  + currentBranch;
    std::string otherBranchPath = refsDir + "/"  + otherBranchName;

    //check if other branch exisits
    if(!fs::exists(otherBranchPath)){
        std::cout<<"Error: Branch '"<< otherBranchName<<"' does not exist.\n";
        return;
    }

    std::string currentHash, otherHash;
    std::ifstream curIn(currentBranchPath);
    std::ifstream othIn(otherBranchPath);
    std::getline(curIn, currentHash);
    std::getline(othIn, otherHash);
    curIn.close();
    othIn.close();

    if(otherHash == "null" || otherHash.empty()){
        std::cout<<"other branch has no commits.\n";
        return;
    }
    std::string lca = findLCA(currentHash, otherHash);
    std::cout<<"LCA: "<< lca<<"\n";

    //Helper lambda to parse commit files
    auto parseCommit =[](const std::string& hash) -> std::map<std::string, std::string>{
        std::map<std::string, std::string> files;
        if(hash == "null" || hash.empty()) return files;

        std::ifstream file(".minigit/commits/" + hash + ".txt");
        std::string line;
        bool section = false;
        while(std::getline(file, line)){
            if(line=="files:"){
                section = true;
                continue;
            }
            if(section && !line.empty()){
                size_t arrow = line.find("->");
                std::string name= line.substr(0, arrow-1);
                std::string blob = line.substr(arrow+3);
                name.erase(0,name.find_first_not_of(" \t\r\n"));
                name.erase(name.find_last_not_of(" \t\r\n") + 1);
                blob.erase(0,blob.find_first_not_of(" \t\r\n"));
                blob.erase(blob.find_last_not_of(" \t\r\n")+1);
                files[name]=blob;
            }
        }
        return files;
    };

    std::map<std::string, std::string> lcaFiles= parseCommit(lca);
    std::map<std::string, std::string> curFiles = parseCommit(currentHash);
    std::map<std::string, std::string> othFiles = parseCommit(otherHash);

    std::map<std::string, std::string> mergedFiles = curFiles;

    for(const auto& entry : othFiles){
        std::string file= entry.first;
        std::string othBlob = entry.second;
        std::string lcaBlob = lcaFiles.count(file) ? lcaFiles[file]:"";
        std::string curBlob = curFiles.count(file) ? curFiles[file]:"";

        if(curFiles.count(file)&& curBlob != othBlob && lcaBlob != curBlob && lcaBlob != othBlob){
            std::cout<< "CONFLICT: both modifies"<<file<<"\n";
            continue; //leave current branch`s version
        }
        //If file does not exist in current, task from other
        if(!curFiles.count(file)){
            mergedFiles[file] = othBlob;
        }
    }
    //create a new merged commit
    time_t now= time(0);
    std::string timestamp= ctime(&now);
    std::string combined ="Merged with" + otherBranchName + timestamp;
    std::string newHash = computeHash(combined);

    std::ofstream out(baseDir + "/commits/" + newHash + ".txt");
    out<<"message: Merge with "<< otherBranchName<<"\n";
    out<<"timestamp: "<<timestamp;
    out<<"parent: "<<currentHash<<"\n";
    out<<"files:\n";
    for(auto& [name, blob]: mergedFiles){
        out<<" "<< name<<" -> "<< blob <<"\n";
    }
    out.close();

    //update HEAD
    std::ofstream updateHead(baseDir + "/" + currentBranch);
    updateHead<< newHash;
    updateHead.close();

    std::cout<< "Merge complete. New commit: "<<newHash<<"\n";
}
void MiniGitRepo::diff(const std::string& commit1, const std::string& commit2) {
    std::string path1=baseDir + "/commits/" + commit1 + ".txt";
    std::string path2=baseDir + "/commits/" + commit2 + ".txt";

    if (!fs::exists(path1) || !fs::exists(path2)) {
        std::cout <<"One or both commits not found.\n";
        return;
    }

    auto parseFiles= [](const std::string& commitPath) -> std::map<std::string, std::string> {
        std::ifstream file(commitPath);
        std::map<std::string, std::string> result;
        std::string line;
        bool readingFiles=false;

        while(std::getline(file, line)) {
            if ( line=="files:") {
                readingFiles=true;
                continue;
            }
            if (readingFiles && !line.empty()) {
                size_t arrow = line.find("->");
                if (arrow != std::string::npos) {
                    std::string filename= line.substr(0,arrow- 1);
                    std::string blob = line.substr(arrow + 3);
                    filename.erase(0,filename.find_first_not_of(" \t\r\n"));
                    filename.erase(filename.find_last_not_of(" \t\r\n") + 1);
                    blob.erase(0,blob.find_first_not_of(" \t\r\n"));
                    blob.erase(blob.find_last_not_of(" \t\r\n") + 1);
                    result[filename]=blob;
                }
            }
        }
        return result;
    };

    auto files1= parseFiles(path1);
    auto files2= parseFiles(path2);

    std::set<std::string> allFiles;
    for (const auto& f: files1) allFiles.insert(f.first);
    for (const auto& f: files2) allFiles.insert(f.first);

    for (const auto& file: allFiles) {
        std::cout << "===File:" <<file << "===\n";

        std::string blob1= files1.count(file) ? files1[file]:"";
        std::string blob2= files2.count(file) ? files2[file]:"";

        std::string file1=objectsDir + "/" + blob1;
        std::string file2=objectsDir + "/" + blob2;

        std::ifstream f1(file1), f2(file2);
        std::string line1, line2;
        int lineNum=1;

        while (getline(f1,line1) || getline(f2,line2)) {
            bool l1= !f1.eof(), l2= !f2.eof();

            if (line1 != line2) {
                std::cout<< "Line "<< lineNum << ":\n";
                if (l1) std::cout<< " -" << line1 <<"\n";
                if (l2) std::cout<< " +" << line2 <<"\n";
            }
            line1.clear(); line2.clear();
            ++lineNum;
        }

        std::cout<< "\n";
    }
}


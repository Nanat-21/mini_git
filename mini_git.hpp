#ifndef MINI_GIT_HPP
#define MINI_GIT_HPP

#include <string>

class MiniGitRepo{
    private:
    const std::string baseDir = ".minigit";
    const std::string objectsDir = ".minigit/objects";
    const std::string refsDir = ".minigit/refs";
    const std::string headFile = ".minigit/HEAD";
    const std::string mainRefFile = ".minigit/refs/main";

    public:
    void init();
};

#endif
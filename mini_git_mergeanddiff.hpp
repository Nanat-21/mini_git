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
                            void add(const std::string& filename);
                                void commit(const std::string& message);
                                    void log();
                                        void branch(const std::string& branchName);
                                            void checkout(const std::string& branchName);
                                                void merge(const std::string& otherBranchName);
                                                    void diff(const std::string& commit1, const std::string& commit2);
                                                    };

                                                    #endif

                                                    
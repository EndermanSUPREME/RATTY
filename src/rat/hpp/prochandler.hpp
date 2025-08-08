#ifndef PROCESS_HANDLER
#define PROCESS_HANDLER

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

class UserProc {
public:
    // thread safe singleton
    static UserProc& getInstance();
    bool CreateBackProc(const std::string& strcmd);
    std::string Show() const;
    bool Run(const std::string& cmd);
    void Close();
private:
    UserProc();
    UserProc(const UserProc&) = delete;
    UserProc& operator=(const UserProc&) = delete;

    // attributes to handle the process within member functions
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;

    // handle references
    HANDLE hStdInRead_;
    HANDLE hStdInWrite_;
    HANDLE hStdOutRead_;
    HANDLE hStdOutWrite_;
};

#endif
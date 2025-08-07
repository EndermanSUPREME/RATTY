#ifndef PROCESS_HANDLER
#define PROCESS_HANDLER

#include <windows.h>
#include <iostream>
#include <string>

class UserProc {
public:
    // thread safe singleton
    static UserProc& getInstance();
    bool CreateBackProc(const std::string& strcmd);
    void Close();
private:
    UserProc();
    UserProc(const UserProc&) = delete;
    UserProc& operator=(const UserProc&) = delete;

    PROCESS_INFORMATION pi;
};

#endif
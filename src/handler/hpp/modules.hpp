#ifndef RATTY_MODULES
#define RATTY_MODULES

#include <iostream>
#include <string>
#include <ratsignal.hpp>

// base class
class Module {
public:
    Module();
    virtual void execute(const SocketHandle& sock) =0;
};

// derived singleton class
class ShellModule : public Module {
public:
    // thread safe singleton
    static ShellModule& getInstance();
    void execute(const SocketHandle& sock);
private:
    ShellModule();
    ShellModule(const ShellModule&) = delete;
    ShellModule& operator=(const ShellModule&) = delete;
};

#endif
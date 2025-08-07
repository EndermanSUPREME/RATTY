#ifndef RATTY_MODULES
#define RATTY_MODULES

#include <iostream>

// base class
class Module {
public:
    Module();
    virtual void execute() =0;
};

// derived singleton class
class ShellModule : public Module {
public:
    // thread safe singleton
    static ShellModule& getInstance();
    void execute();
private:
    ShellModule(){};
    ShellModule(const ShellModule&) = delete;
    ShellModule& operator=(const ShellModule&) = delete;
};

#endif
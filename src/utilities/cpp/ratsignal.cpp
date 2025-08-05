#include <ratsignal.hpp>

// convert a string into a hex string
std::string ToHex(const std::string& input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}

// convert hex string to string
std::string FromHex(const std::string& hex) {
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
std::string FormatHex(const std::string& input) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : input) {
        oss << "\\x" << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}

RatPacket::RatPacket(std::string content, MsgType msgType): msg(content), type(msgType) {
    size = (unsigned int)content.size();
}

RatPacket::RatPacket(const char* frame) {
    std::string frameStr(frame);
    bool isValid = Validate(frameStr);

    if (isValid) {
        // extract frame segments
        std::vector<std::string> segments;
        std::string segment;

        for (size_t i = 2; i < frameStr.size()-2; ++i) {
            // fetch hex byte
            std::string byteString = frameStr.substr(i, 1);
            
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
                size = std::stoi(FromHex(segments[0]));
                type = (MsgType)std::stoi(FromHex(segments[1]));
                msg = FromHex(segments[2]);
            } catch (const std::exception& ex) {
                std::cerr << "Exception: " << ex.what() << std::endl;
            }
        } else {
            std::cerr << "-- Error extracting segments!" << std::endl;
        }
    } else {
        std::cerr << "-- Invalid frame data!" << std::endl;
    }
}

bool RatPacket::Validate(const std::string& frame) const {
    std::string start = frame.substr(0,2); // two indexes give us a hex chunk ie: \xbe\xef
    std::string end = frame.substr(frame.length()-2,2);

    bool validStart = (FormatHex(start) == FormatHex("\xDE\xAD"));
    bool validEnd = (FormatHex(end) == FormatHex("\xBE\xEF"));

    int delimitCount = 0;
    for (int i = 2; i < frame.length()-2; ++i) {
        // Each hex byte (\xA1) is a single index of the frame string
        std::string byteString = FormatHex(frame.substr(i, 1));
        if (byteString == FormatHex("\x7C")) ++delimitCount;
    }

    return validStart && validEnd && (delimitCount == 4);
}

std::string RatPacket::Frame() const {
    std::string frame;

    frame += "\xDE\xAD";
    frame += "\x7C"; // delimiter

    frame += ToHex(std::to_string(size));
    frame += "\x7C"; // delimiter

    frame += ToHex(std::to_string((int)type));
    frame += "\x7C"; // delimiter

    frame += ToHex(msg);
    frame += "\x7C"; // delimiter
    frame += "\xBE\xEF";

    return frame;
}

// gets size of packet frame for socket sending
int RatPacket::SizeOf() const {
    return static_cast<int>(strlen(Frame().c_str()));
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

void RatPacket::SetPacketMessage(std::string nMsg) {
    if (msg == nMsg) return;
    msg = nMsg;
    size = (unsigned int)nMsg.size();
}
void RatPacket::SetType(MsgType nType) {
    if (type == nType) return;
    type = nType;
}
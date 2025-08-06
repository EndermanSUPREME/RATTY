#include <ratsignal.hpp>

bool TestPacketCreation() {
    // create packet obj
    RatPacket packet("This is a test!", MsgType::NONE);
    
    // convert packet frame string into packet obj
    RatPacket testPacket(packet.Frame());

    bool correctSize = testPacket.Length() == packet.Length();
    bool correctType = testPacket.GetType() == packet.GetType();
    bool correctMsg = testPacket.GetPacketMessage() == packet.GetPacketMessage();

    if (!(correctSize && correctType && correctMsg)) {
        std::cerr << std::boolalpha << "RESULTS: " << 
            correctSize << " && " << correctType << " && " << correctMsg
            << std::endl;
    }

    testPacket.VisualizePacket();

    return correctSize && correctType && correctMsg;
}

void TestFailed(const std::string name) {
    std::cout << "[-] Test " << name << " Failed!" << std::endl;
    std::exit(1);
}

int main() {
    std::cout << "[+] Running Tests. . ." << std::endl;
    if (!TestPacketCreation()) {
        TestFailed("TestPacketCreation");
    }
    std::cout << "[+] Tests Passed!" << std::endl;
}
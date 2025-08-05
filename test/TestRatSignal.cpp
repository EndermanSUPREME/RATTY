#include <ratsignal.hpp>

bool TestPacketCreation() {
    const char* testFrame = "\xde\xad\x7c\x33\x31\x33\x37\x7c\x33\x30\x7c\x34\x38\x36\x35\x36\x43\x36\x43\x36\x46\x32\x30\x36\x36\x37\x32\x36\x46\x36\x44\x32\x30\x36\x33\x36\x43\x36\x39\x36\x35\x36\x45\x37\x34\x7c\xbe\xef";
    RatPacket testPacket(testFrame);

    bool correctSize = testPacket.Length() == 17;
    bool correctType = testPacket.GetType() == MsgType::EXEC;
    bool correctMsg = testPacket.GetMessage() == "Hello from client";

    if (!(correctSize && correctType && correctMsg)) {
        std::cerr << std::boolalpha << "RESULTS: " << 
            correctSize << " && " << correctType << " && " << correctMsg
            << std::endl;
    }

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
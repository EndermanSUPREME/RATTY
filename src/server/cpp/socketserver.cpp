#include <socketserver.hpp>

Server::Server(std::string lhost, int lport): LHOST(lhost), LPORT(lport), sock_conn(INVALID_SOCKET_HANDLE) {
    if (!InitializeServer()) {
        std::cout << "[-] Server could not be created!" << std::endl;
        LPORT = -1;
    } else {
        std::cout << "[+] Server created!" << std::endl;
    }
}

// close sockets on obj dtor
Server::~Server() {
#ifdef _WIN32
    closesocket(sock_conn);
    closesocket(sock_server);
    WSACleanup();
#else
    close(sock_conn);
    close(sock_server);
#endif
}

// if LPORT is greater than 0 the server is online
bool Server::ServerUp() {
    return (LPORT > 0);
}

bool Server::InitializeServer() {
#ifdef _WIN32
    WSADATA wsaData;

    sockaddr_in address;
    int addrlen = sizeof(address);
    const int PORT = LPORT;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[-] WSAStartup failed\n";
        return false;
    }

    // Create socket
    sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_server == INVALID_SOCKET_HANDLE) {
        std::cerr << "[-] Socket failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return false;
    }

    // Bind
    address.sin_family = AF_INET;
    inet_pton(AF_INET, LHOST.c_str(), &address.sin_addr);
    address.sin_port = htons(PORT);

    if (bind(sock_server, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "[-] Bind failed: " << WSAGetLastError() << "\n";
        closesocket(sock_server);
        WSACleanup();
        return false;
    }

    // Listen
    if (listen(sock_server, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[-] Listen failed: " << WSAGetLastError() << "\n";
        closesocket(sock_server);
        WSACleanup();
        return false;
    }

    std::cout << "Server Active on " << LHOST << ":" << LPORT << "\n";
    std::cout << "Waiting for connection...\n";

    sock_conn = accept(sock_server, (sockaddr*)&address, &addrlen);
    if (sock_conn == INVALID_SOCKET_HANDLE) {
        std::cerr << "[-] Accept failed: " << WSAGetLastError() << "\n";
        closesocket(sock_server);
        WSACleanup();
        return false;
    }

    return true;
#else
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server == -1)
    {
        std::cerr << "[-] Failed to create socket server!" << std::endl;
        return false;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(LPORT);
    
    if (inet_pton(AF_INET, LHOST.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "[-] Address invalid!" << std::endl;
        close(sock_server);
        return false;
    }

    if (bind(sock_server, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "[-] Failed to Bind socket server!" << std::endl;
        close(sock_server);
        return false;
    }

    // Listen for incoming connections
    if (listen(sock_server, 1) == -1)
    {
        std::cerr << "[-] Failed to listen on server-side!" << std::endl;
        close(sock_server);
        return false;
    }

    std::cout << "Server Active on " << LHOST << ":" << LPORT << "\n";
    std::cout << "Waiting for connection...\n";

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    sock_conn = accept(sock_server, (struct sockaddr *)&client_addr, &client_addr_len);

    if (sock_conn == INVALID_SOCKET_HANDLE) {
        std::cerr << "[-] Failed to accept incoming connection!" << std::endl;
        close(sock_server);
        return false;
    }

    return true;
#endif
}

// return an immutable reference
SocketHandle* Server::GetConnectionSocket() {
    return &sock_conn;
}
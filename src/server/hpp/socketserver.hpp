#ifndef RATTY_SOCKET_SERVER
#define RATTY_SOCKET_SERVER

#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <utility>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>

    // typedef alias to link multiple types together
    typedef SOCKET SocketHandle; // WinSocks are type SOCKETS
    const SocketHandle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
#else
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> 
    #include <sys/types.h>
    #include <sys/socket.h>

    // typedef alias to link multiple types together
    typedef int SocketHandle; // Unix sockets are type int
    const SocketHandle INVALID_SOCKET_HANDLE = -1;
#endif

class Server {
public:
    Server(std::string lhost, int lport);
    ~Server();
    bool InitializeServer();
    bool ServerUp();
    SocketHandle* GetConnectionSocket();
private:
    SocketHandle sock_conn;
    SocketHandle sock_server;

    std::string LHOST;
    int LPORT;
};

#endif
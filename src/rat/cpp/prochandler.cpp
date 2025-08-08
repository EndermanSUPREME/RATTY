#include <prochandler.hpp>

UserProc::UserProc(){}
UserProc& UserProc::getInstance() {
    static UserProc instance;
    return instance;
}

bool UserProc::CreateBackProc(const std::string& strcmd) {
    std::cout << "[*] Setting up Process. . ." << std::endl;
    si = { 0 };
    pi = { 0 };

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    // Create pipes for STDOUT
    if (!CreatePipe(&hStdOutRead_, &hStdOutWrite_, &sa, 0) ||
        !SetHandleInformation(hStdOutRead_, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "[-] Failed to create stdout pipe." << std::endl;
        return false;
    }

    // Create pipes for STDIN
    if (!CreatePipe(&hStdInRead_, &hStdInWrite_, &sa, 0) ||
        !SetHandleInformation(hStdInWrite_, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "[-] Failed to create stdin pipe." << std::endl;
        return false;
    }

    // Setup STARTUPINFO
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // bitwise usage
    si.wShowWindow = SW_SHOWNORMAL;

    // Connect the pipes
    si.hStdInput  = hStdInRead_;
    si.hStdOutput = hStdOutWrite_;
    si.hStdError  = hStdOutWrite_;

    const char* cmd = strcmd.c_str();

    bool success = CreateProcessA(
        nullptr,                // lpApplicationName
        (LPSTR)cmd,             // lpCommandLine
        nullptr,                // lpProcessAttributes
        nullptr,                // lpThreadAttributes
        TRUE,                   // bInheritHandles must be TRUE to use pipe handles
        CREATE_NEW_CONSOLE,     // dwCreationFlags
        nullptr,                // lpEnvironment
        nullptr,                // lpCurrentDirectory
        &si,                    // lpStartupInfo
        &pi                     // lpProcessInformation
    );

    if (!success) {
        std::cerr << "[-] Failed to create process. Error: " << GetLastError() << std::endl;
        Close();
        return false;
    }

    std::cout << "[+] Process created in background. PID: " << pi.dwProcessId << std::endl;
    return true;
}

std::string UserProc::Show() const {
    std::cout << "[*] Attempting to print hStdOutRead pipe contents. . ." << std::endl;
    // Read the output into dynamic buffer
    std::vector<char> buffer;
    DWORD bytesRead;
    int readAttempts = 0;

    while (true) {
        DWORD bytesAvailable = 0;

        if (!PeekNamedPipe(hStdOutRead_, NULL, 0, NULL, &bytesAvailable, NULL)) {
            std::cerr << "PeekNamedPipe failed: " << GetLastError() << std::endl;
            break;
        }

        if (bytesAvailable == 0) {
            ++readAttempts;

            // after too many attempts assume no more data can be read
            if (readAttempts >= 20) break;
            
            // No data right now
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        readAttempts = 0;  // reset when data is found

        buffer.resize(buffer.size() + 1024);  // grow buffer
        BOOL success = ReadFile(hStdOutRead_, buffer.data() + buffer.size() - 1024, 1024, &bytesRead, NULL);
        if (!success || bytesRead == 0)
            break;

        buffer.resize(buffer.size() - (1024 - bytesRead)); // shrink if less was read
    }

    // save output
    std::cout << "[+] Successfully read hStdOutRead pipe contents!" << std::endl;
    std::string output(buffer.begin(), buffer.end());
    return output;
}

bool UserProc::Run(const std::string& cmd) {
    // newline signals to terminal end-of-input
    std::string fullcmd = cmd + "\n";

    // like pulling from stdout handle via ReadFile
    // to send input through the HANDLE you use WriteFile
    DWORD bytesWritten = 0;
    BOOL success = WriteFile(
        hStdInWrite_,
        fullcmd.c_str(),
        static_cast<DWORD>(fullcmd.size()),
        &bytesWritten,
        NULL
    );

    if (!success || bytesWritten != fullcmd.size()) {
        std::cerr << "[-] Failed to write command to stdin pipe. Error: " << GetLastError() << std::endl;
        return false;
    }

    // helps push the input immediently via a flush
    FlushFileBuffers(hStdInWrite_);
    return true;
}

void UserProc::Close() {
    std::cout << "[*] Closing Process. . ." << std::endl;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
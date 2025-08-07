#include <prochandler.hpp>

UserProc::UserProc(){}
UserProc& UserProc::getInstance() {
    static UserProc instance;
    return instance;
}

bool UserProc::CreateBackProc(const std::string& strcmd) {
    std::cout << "[*] Setting up Process. . ." << std::endl;
    STARTUPINFOA si = { 0 };
    pi = { 0 };

    // set proc metadata
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    const char* cmd = strcmd.c_str();

    bool success = CreateProcessA(
        nullptr,                  // lpApplicationName
        (LPSTR)cmd,               // lpCommandLine
        nullptr,                  // lpProcessAttributes
        nullptr,                  // lpThreadAttributes
        FALSE,                    // bInheritHandles
        CREATE_NO_WINDOW,         // dwCreationFlags
        nullptr,                  // lpEnvironment
        nullptr,                  // lpCurrentDirectory
        &si,                      // lpStartupInfo
        &pi                       // lpProcessInformation
    );

    if (!success) {
        std::cerr << "[-] Failed to create process. Error: " << GetLastError() << std::endl;
        Close();
        return false;
    }

    std::cout << "[+] Process created in background. PID: " << pi.dwProcessId << std::endl;
    return true;
}

void UserProc::Close() {
    std::cout << "[*] Closing Process. . ." << std::endl;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
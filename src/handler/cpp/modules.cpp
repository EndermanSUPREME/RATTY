#include <modules.hpp>

Module::Module() {}

ShellModule& ShellModule::getInstance() {
    static ShellModule instance;
    return instance;
}

void ShellModule::execute() {
    std::cout << "[*] Starting up Shell Module. . ." << std::endl;
    // placement
    std::string input;
    while (input != "exit") {
        printf("SHELL >$ ");
        std::cin >> input;
    }
    std::cout << "[*] Closing Shell Module. . ." << std::endl;
}
#include <rathandler.hpp>

int main(int argc, char** argv) {
    CLI::App app{"RATTY Handler (Specialized RAT Handler)"};
    
    std::string LHOST;
    int LPORT;
    
    app.add_option("-l, --lhost", LHOST, "Handler Listening IP")
        ->required();
    app.add_option("-p, --port", LPORT, "Handler Listening Port")
        ->required();
    
    CLI11_PARSE(app, argc, argv);

    Handler ratHandler(LHOST, LPORT);
    return 0;
}
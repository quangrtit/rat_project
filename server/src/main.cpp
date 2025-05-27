#include "Server.hpp"
#include "Constants.hpp"
#include <iostream>
#include <string>
#include <csignal>

// namespace {
//     volatile std::sig_atomic_t g_signal_status = 0;

//     void signal_handler(int signal) {
//         g_signal_status = signal;
//     }
// }

int main(int argc, char* argv[]) 
{
    uint16_t port = Rat::DEFAULT_PORT;

    if (argc > 2) 
    {
        std::cerr << "Usage: " << argv[0] << " [<port>]" << std::endl;
        std::cerr << "  Default port: " << port << std::endl;
        return 1;
    }
    if (argc == 2) 
    {
        try 
        {
            port = std::stoi(argv[1]);
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Invalid port number: " << argv[1] << std::endl;
            return 1;
        }
    }

    try 
    {
        // Đăng ký xử lý tín hiệu Ctrl+C
        // std::signal(SIGINT, signal_handler);

        std::cout << "Server starting on port " << port << std::endl;
        std::cout << "Enter messages to broadcast to all clients (Ctrl+C to quit)." << std::endl;
        Rat::Server server(port);
        server.start();

        // Kiểm tra tín hiệu Ctrl+C
        // if (g_signal_status == SIGINT) {
        //     std::cout << "\nReceived Ctrl+C, shutting down server..." << std::endl;
        //     server.stop();
        // }
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
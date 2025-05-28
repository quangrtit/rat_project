#include "Server.hpp"
#include "Constants.hpp"
#include <iostream>
#include <string>
#include <csignal>

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
        std::cout << "Server starting on port " << port << std::endl;
        std::cout << "Enter packet type to broadcast to all clients (Ctrl+C to quit)." << std::endl;
        Rat::Server server(port);
        server.start();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
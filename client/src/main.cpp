#include "Client.hpp"
#include "Constants.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) 
{
    std::string server_ip = "127.0.0.1";
    uint16_t port = Rat::DEFAULT_PORT;

    if (argc > 3) 
    {
        std::cerr << "Usage: " << argv[0] << " [<server_ip> [<port>]]" << std::endl;
        std::cerr << "  Default server_ip: " << server_ip << ", port: " << port << std::endl;
        return 1;
    }
    if (argc >= 2) 
    {
        server_ip = argv[1];
    }
    if (argc == 3) 
    {
        try 
        {
            port = std::stoi(argv[2]);
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Invalid port number: " << argv[2] << std::endl;
            return 1;
        }
    }

    try 
    {
        std::cout << "Client starting, connecting to " << server_ip << ":" << port << std::endl;
        std::cout << "Enter packet type to send to server (Ctrl+D or empty line to quit)." << std::endl;
        Rat::Client client(server_ip, port);
        client.start();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
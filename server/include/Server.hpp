#ifndef RAT_SERVER_HPP
#define RAT_SERVER_HPP

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include "NetworkManager.hpp"

namespace Rat 
{

    class Server 
    {
    public:
        Server(uint16_t port);
        ~Server();
        void start();
        void stop();
    private:
        void acceptConnections();
        void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> client);
        void sendCommandToClient(std::shared_ptr<boost::asio::ip::tcp::socket> client, const Command& command);
        void handleUserInput();

        boost::asio::io_context io_context_;
        boost::asio::ip::tcp::acceptor acceptor_;
        std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> clients_;
        NetworkManager networkManager_;
        std::thread input_thread_;
    };

} // namespace Rat

#endif // RAT_SERVER_HPP
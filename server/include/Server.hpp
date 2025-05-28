#ifndef RAT_SERVER_HPP
#define RAT_SERVER_HPP

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <set>
#include <unordered_map>
#include <cstdint>
#include "NetworkManager.hpp"
#include "Packet.pb.h" // Thêm header Protobuf

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
        void initListClientID(const std::string &path = "../config/list_client.txt");
        void acceptConnections();
        void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> client);
        void sendCommandToClient(std::shared_ptr<boost::asio::ip::tcp::socket> client, const rat::Packet& packet);
        void handleUserInput();

        std::set<uint64_t> client_id_;
        boost::asio::io_context io_context_;
        boost::asio::ip::tcp::acceptor acceptor_;
        std::unordered_map<std::shared_ptr<boost::asio::ip::tcp::socket>, uint64_t> clients_;
        NetworkManager networkManager_;
        std::thread input_thread_;
    };

} // namespace Rat

#endif // RAT_SERVER_HPP
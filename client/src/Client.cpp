#include "Client.hpp"
#include <iostream>
#include <thread>

namespace Rat 
{

    Client::Client(const std::string& host, uint16_t port)
        : socket_(std::make_shared<boost::asio::ip::tcp::socket>(io_context_)),
        timer_(io_context_),
        host_(host),
        port_(port),
        stopping_(false) 
        {}

    Client::~Client() 
    {
        stopping_ = true;
        socket_->close();
        if (input_thread_.joinable()) 
        {
            input_thread_.join();
        }
    }

    void Client::start() 
    {
        tryConnect();
        input_thread_ = std::thread(&Client::handleUserInput, this);
        io_context_.run();
    }

    void Client::tryConnect() 
    {
        if (stopping_) return;

        socket_ = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        boost::asio::ip::tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));

        boost::asio::async_connect(*socket_, endpoints,
            [this](const boost::system::error_code& ec, boost::asio::ip::tcp::endpoint) 
            {
                if (!ec) 
                {
                    std::cout << "Connected to server: " << host_ << ":" << port_ << std::endl;
                    handleCommands();
                } 
                else 
                {
                    std::cerr << "Connection failed: " << ec.message() << std::endl;
                    scheduleReconnect();
                }
            });
    }

    void Client::scheduleReconnect() 
    {
        if (stopping_) return;

        timer_.expires_from_now(boost::posix_time::seconds(5));
        timer_.async_wait([this](const boost::system::error_code& ec) 
        {
            if (!ec && !stopping_) 
            {
                std::cout << "Retrying connection to server..." << std::endl;
                tryConnect();
            }
        });
    }

    void Client::handleCommands() 
    {
        if (stopping_) return;

        networkManager_.receive(socket_,
            [this](const Response& response, const boost::system::error_code& ec) 
            {
                if (ec) 
                {
                    std::cerr << "Receive error: " << ec.message() << std::endl;
                    scheduleReconnect();
                    return;
                }
                if (response.success) 
                {
                    std::cout << "Server: " << response.data << std::endl;
                } 
                else 
                {
                    std::cerr << "Error from server: " << response.error_message << std::endl;
                }
                boost::asio::post(io_context_, [this]() { handleCommands(); });
            });
    }

    void Client::handleUserInput() 
    {
        std::string input;
        // std::cout << "Enter input from client: ";
        while (std::getline(std::cin, input)) 
        {
            if (input.empty() || stopping_) continue;
            Command cmd(CommandType::LIST_PROCESSES, input);
            networkManager_.send(socket_, cmd,
                [](const boost::system::error_code& ec) 
                {
                    if (ec) 
                    {
                        std::cerr << "Send error: " << ec.message() << std::endl;
                    }
                });
            std::cout << "Client sent: " << input << std::endl;
            // std::cout << "Enter input from client: ";
        }
    }

} // namespace Rat
#include "Server.hpp"
#include <iostream>
#include <thread>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
namespace Rat 
{
    std::string md5HashString(const std::string& input) {
        unsigned char md_value[EVP_MAX_MD_SIZE];
        unsigned int md_len = 0;

        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(mdctx, EVP_md5(), nullptr);
        EVP_DigestUpdate(mdctx, input.data(), input.size());
        EVP_DigestFinal_ex(mdctx, md_value, &md_len);
        EVP_MD_CTX_free(mdctx);

        std::stringstream ss;
        for (unsigned int i = 0; i < md_len; ++i)
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)md_value[i];
        return ss.str();
    }
    Server::Server(uint16_t port)
        : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) 
        {}

    Server::~Server() 
    {
        stop();
    }

    void Server::start() 
    {
        acceptConnections();
        input_thread_ = std::thread(&Server::handleUserInput, this);
        io_context_.run();
    }

    void Server::stop() 
    {
        io_context_.stop();
        clients_.clear();
        if (input_thread_.joinable()) 
        {
            input_thread_.join();
        }
    }

    void Server::acceptConnections() 
    {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        acceptor_.async_accept(*socket,
            [this, socket](const boost::system::error_code& ec) 
            {
                if (!ec) 
                {
                    clients_.push_back(socket);
                    std::string client_id = std::to_string(clients_.back()->native_handle());
                    // std::cout << "this id: " << clients_.back()->native_handle() << std::endl;
                    std::cout << "New client connected: " << socket->remote_endpoint().address().to_string() + "_" + md5HashString(client_id)<< std::endl;
                    handleClient(socket);
                } 
                else 
                {
                    std::cerr << "Accept error: " << ec.message() << std::endl;
                }
                boost::asio::post(io_context_, [this]() { acceptConnections(); });
            });
    }

    void Server::handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> client) 
    {
        networkManager_.receive(client,
            [this, client](const Response& response, const boost::system::error_code& ec) 
            {
                if (ec) 
                {
                    std::cerr << "Client error: " << ec.message() << std::endl;
                    clients_.erase(
                        std::remove(clients_.begin(), clients_.end(), client), clients_.end());
                    return;
                }
                if (response.success) 
                {
                    std::string client_id = std::to_string(client->native_handle());
                    std::cout << "Client (" << client->remote_endpoint().address().to_string() + "_" + md5HashString(client_id) << "): "
                            << response.data << std::endl;
                } 
                else 
                {
                    std::cerr << "Error from client: " << response.error_message << std::endl;
                }
                boost::asio::post(io_context_, [this, client]() { handleClient(client); });
            });
    }

    void Server::sendCommandToClient(std::shared_ptr<boost::asio::ip::tcp::socket> client, const Command& command) 
    {
        networkManager_.send(client, command,
            [](const boost::system::error_code& ec) 
            {
                if (ec) 
                {
                    std::cerr << "Send error: " << ec.message() << std::endl;
                }
            });
    }

    void Server::handleUserInput() 
    {
        std::string input;
        // std::cout << "Enter input from server: ";
        while (std::getline(std::cin, input)) 
        {
            if (input.empty()) continue;
            Command cmd(CommandType::LIST_FILES, input);
            for (auto& client : clients_) 
            {
                sendCommandToClient(client, cmd);
            }
            std::cout << "Server sent: " << input << std::endl;
            // std::cout << "Enter input from server: ";
        }
    }

} // namespace Rat
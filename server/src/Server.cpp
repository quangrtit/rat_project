#include "Server.hpp"
#include <iostream>
#include <thread>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <random> // Để tạo dữ liệu ngẫu nhiên
#include <fstream>

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

    // Hàm tạo dữ liệu ngẫu nhiên
    std::string generateRandomData(size_t length) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(33, 126); // Ký tự in được (ASCII 33-126)
        std::string data;
        for (size_t i = 0; i < length; ++i) {
            data += static_cast<char>(dis(gen));
        }
        return data;
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
        initListClientID();
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
    void Server::initListClientID(const std::string &path)
    {
        std::ifstream file_config(path);
        if(!file_config.is_open())
        {
            std::cout << "Open file not success!\n" << std::endl;
            return;
        }
        std::string client_id;
        while(std::getline(file_config, client_id)) 
        {
            try {
                uint64_t number_client_id = std::stol(client_id);
                // std::cout <<  "Debug: " << number_client_id << std::endl;
                client_id_.insert(number_client_id);
            } 
            catch (const std::invalid_argument& e) {
                std::cerr << "ERROR: Invalid number!\n";
            } 
        }
        file_config.close();
    }

    void Server::acceptConnections() 
    {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        acceptor_.async_accept(*socket,
            [this, socket](const boost::system::error_code& ec) 
            {
                if (!ec) 
                {
                    clients_.insert({socket, -1});
                    std::string client_id = std::to_string(socket->native_handle());
                    std::cout << "New client connected: " << socket->remote_endpoint().address().to_string() + "_" + md5HashString(client_id) << std::endl;
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
            [this, client](const rat::Packet& packet, const boost::system::error_code& ec) 
            {
                if (ec) 
                {
                    std::cerr << "Client error: " << ec.message() << std::endl;
                    clients_.erase(
                        client);
                    return;
                }
                if (packet.has_command_data() && packet.command_data().command().size()) 
                {
                    std::cout << "Debug IDENTIFY: " << packet.destination_id() << " " << packet.source_id() << " " << packet.command_data().command() << std::endl;
                    // set id for client 
                    uint64_t source_id = -1;
                    if (packet.type() == rat::Packet::IDENTIFY) 
                    {
                        try 
                        {
                            source_id = stol(packet.source_id());
                        }
                        catch (const std::invalid_argument& e) 
                        {
                            std::cerr << "ERROR: Invalid number!\n";
                        } 
                        auto exist_client = client_id_.find(source_id);
                        if (exist_client != client_id_.end())
                        {
                            clients_.insert({client, source_id});
                        }
                        else 
                        {
                            if (client_id_.empty())
                            {
                                source_id = 0;
                            }
                            else
                            {
                                source_id = *client_id_.rbegin() + 1;
                            }
                            rat::Packet packet;
                            packet.set_packet_id("server_" + std::to_string(std::rand()));
                            packet.set_source_id("-1");
                            packet.set_destination_id(std::to_string(source_id));
                            packet.set_type(rat::Packet::IDENTIFY);

                            auto* command = packet.mutable_command_data();
                            std::string random_data = generateRandomData(10);
                            command->set_command(random_data);
                            
                            sendCommandToClient(client, packet); // send id for client 
                            // insert id 
                            client_id_.insert(source_id);
                            clients_.insert({client, source_id});
                            // insert id in file 
                            std::ofstream file_config("../config/list_client.txt", std::ios::app);
                            if(!file_config.is_open())
                            {
                                std::cout << "Open file not success!\n" << std::endl;
                                return;
                            }
                            file_config << source_id << "\n";
                            file_config.close();
                        }
                    }
                    std::string client_id = std::to_string(client->native_handle());
                    std::cout << "Client (" << client->remote_endpoint().address().to_string() + "_" + md5HashString(client_id) << "): "
                            << packet.command_data().command() << std::endl;
                } 
                else if (packet.has_command_data() && !packet.command_data().command().size()) 
                {
                    std::cerr << "Error from client: " << std::endl;
                }
                boost::asio::post(io_context_, [this, client]() { handleClient(client); });
            });
    }

    void Server::sendCommandToClient(std::shared_ptr<boost::asio::ip::tcp::socket> client, const rat::Packet& packet) 
    {
        networkManager_.send(client, packet,
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
        std::string input_type;
        std::cout << "Enter packet type (LIST_FILES, LIST_PROCESSES, READ_FILE, KILL_PROCESS): ";
        while (std::getline(std::cin, input_type)) 
        {
            if (input_type.empty()) continue;

            rat::Packet packet;
            packet.set_packet_id("server_" + std::to_string(std::rand()));
            packet.set_source_id("server");
            packet.set_destination_id("all_clients");

            // Gán loại packet dựa trên input
            if (input_type == "LIST_FILES") {
                packet.set_type(rat::Packet::LIST_FILES);
            } else if (input_type == "LIST_PROCESSES") {
                packet.set_type(rat::Packet::LIST_PROCESSES);
            } else if (input_type == "READ_FILE") {
                packet.set_type(rat::Packet::READ_FILE);
            } else if (input_type == "KILL_PROCESS") {
                packet.set_type(rat::Packet::KILL_PROCESS);
            } else {
                std::cout << "Unknown type, use: LIST_FILES, LIST_PROCESSES, READ_FILE, KILL_PROCESS" << std::endl;
                std::cout << "Enter packet type: ";
                continue;
            }

            // Tạo dữ liệu ngẫu nhiên cho command
            auto* command = packet.mutable_command_data();
            std::string random_data = generateRandomData(10); // Dữ liệu 10 ký tự ngẫu nhiên
            command->set_command(random_data);

            for (auto& client : clients_) 
            {
                sendCommandToClient(client.first, packet);
            }
            std::cout << "Server sent: Type=" << input_type << ", Data=" << random_data << std::endl;
            std::cout << "Enter packet type: ";
        }
    }

} // namespace Rat
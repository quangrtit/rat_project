#include "Server.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>

namespace Rat 
{

    std::string md5HashString(const std::string& input) 
    {
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

    std::string generateRandomData(size_t length) 
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(33, 126);
        std::string data;
        for (size_t i = 0; i < length; ++i) {
            data += static_cast<char>(dis(gen));
        }
        return data;
    }

    Server::Server(uint16_t port)
        : networkManager_(), acceptor_(networkManager_.get_io_context(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
        
    {
        networkManager_.setup_ssl_context(true, "./server.crt", "./server.key", "./ca.crt");
    }

    Server::~Server() 
    {
        stop();
    }

    void Server::start() 
    {
        initListClientID();
        acceptConnections();
        input_thread_ = std::thread(&Server::handleUserInput, this);
        networkManager_.get_io_context().run();
    }

    void Server::stop() 
    {
        networkManager_.get_io_context().stop();
        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            clients_.clear();
        }
        {
            std::lock_guard<std::mutex> lock(client_id_mutex_);
            client_id_.clear();
        }
        if (input_thread_.joinable()) input_thread_.join();
    }

    void Server::initListClientID(const std::string& path) 
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::app);
        if (!file.is_open()) 
        {
            std::cout << "Cannot open file: " << path << "\n";
            return;
        }
        std::string client_id;
        while (std::getline(file, client_id)) 
        {
            try 
            {
                client_id_.insert(std::stol(client_id));
            } 
            catch (const std::exception& e) 
            {
                std::cout << "Invalid ID: " << e.what() << "\n";
            }
        }
        file.close();
    }

    void Server::acceptConnections() 
    {
        auto socket = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
            networkManager_.get_io_context(), networkManager_.get_ssl_context());

        acceptor_.async_accept(socket->lowest_layer(),
            [this, socket](const boost::system::error_code& ec) {
                if (!ec) {
                    socket->set_verify_mode(boost::asio::ssl::verify_peer);
                    socket->async_handshake(boost::asio::ssl::stream_base::server,
                        [this, socket](const boost::system::error_code& ec) {
                            if (!ec) {
                                std::cout << "New client connected: " << socket->lowest_layer().remote_endpoint().address().to_string() << "\n";
                                handleClient(socket);
                            } else {
                                std::cout << "Handshake error: " << ec.message() << "\n";
                            }
                            boost::asio::post(networkManager_.get_io_context(), [this]() { acceptConnections(); });
                            // acceptConnections();
                        });
                } else {
                    std::cout << "Accept error: " << ec.message() << "\n";
                    boost::asio::post(networkManager_.get_io_context(), [this]() { acceptConnections(); });
                    // acceptConnections();
                }
            });
    }

    
    void Server::handleClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client) {
        networkManager_.receive(client,
            [this, client](const rat::Packet& packet, const boost::system::error_code& ec) 
            {

                if (ec) 
                {
                    std::cout << "Client error: " << ec.message() << "\n";
                    client->lowest_layer().close();
                    removeClient(client);
                    return;
                }
                if (packet.has_chunked_data() && packet.type() == rat::Packet::STATIC_ID) 
                {
                    
                    uint64_t source_id = parseStaticIdPayload(packet.chunked_data().payload());
                    if (client_id_.find(source_id) != client_id_.end()) 
                    {   
                        updateClientWithExistingId(client, source_id);
                        // std::cout << "Debug update: " << source_id << std::endl;
                    } 
                    else 
                    {
                        assignNewClientId(client, source_id);
                        saveClientIdsToFile(source_id);
                        // std::cout << "Debug assign: " << source_id << std::endl;
                    }
                    
                    // std::cout << "Debug: clients: ";
                    // for(auto x: clients_) {std::cout << x.first << " - " << x.second << std::endl;}
                }
                std::lock_guard<std::mutex> lock(client_mutex_);
                
                auto it = clients_.find(client);
                if (it != clients_.end()) 
                {
                    if(it->second == uint64_t(-1)) {std::cout << "Debug set size: " << clients_.size() << " " << packet.type() << std::endl;}
                    std::cout << "Client (" << client->lowest_layer().remote_endpoint().address().to_string() + "_" + std::to_string(clients_[client]) << "): "
                            << packet.chunked_data().payload() << "\n";
                }
                boost::asio::post(networkManager_.get_io_context(), [this, client]() { handleClient(client); });
            });
    }

    void Server::sendCommandToClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client, const rat::Packet& packet) 
    {
        networkManager_.send(client, packet, [](const boost::system::error_code& ec) 
        {
            if (ec) std::cout << "Send error: " << ec.message() << "\n";
        });
    }

    void Server::handleUserInput() 
    {
        std::string input;
        std::cout << "Enter packet type (LIST_FILES, LIST_PROCESSES, READ_FILE, KILL_PROCESS): ";
        while (std::getline(std::cin, input)) 
        {
            rat::Packet packet;
            packet.set_packet_id("server_" + std::to_string(std::rand()));
            packet.set_source_id("server");
            packet.set_destination_id("all_clients");

            if (input == "LIST_FILES") 
            {
                packet.set_type(rat::Packet::LIST_FILES);
            } 
            else if (input == "LIST_PROCESSES") 
            {
                packet.set_type(rat::Packet::LIST_PROCESSES);
            } 
            else if (input == "READ_FILE") 
            {
                packet.set_type(rat::Packet::READ_FILE);
            } 
            else if (input == "KILL_PROCESS") 
            {
                packet.set_type(rat::Packet::KILL_PROCESS);
            } 
            else 
            {
                std::cout << "Unknown type, use: LIST_FILES, LIST_PROCESSES, READ_FILE, KILL_PROCESS\n";
                std::cout << "Enter packet type: ";
                continue;
            }

            auto* chunked = packet.mutable_chunked_data();
            chunked->set_payload(generateRandomData(10));

            for (auto& client : clients_) 
            {
                sendCommandToClient(client.first, packet);
            }
            std::cout << "Server sent: Type=" << input << ", Data=" << chunked->payload() << "\n";
            std::cout << "Enter packet type: ";
        }
    }

    void Server::addClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client, uint64_t client_id) 
    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        if (clients_.size() >= MAX_CLIENT) 
        {
            std::cout << "Error: Maximum client limit reached!\n";
            if (client) client->lowest_layer().close();
            return;
        }
        clients_[client] = client_id;
        // clients_.insert({client, client_id});
    }

    void Server::removeClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client) 
    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        clients_.erase(client);
    }

    uint64_t Server::parseStaticIdPayload(const std::string& payload) 
    {
        try 
        {
            return std::stol(payload);
        } 
        catch (const std::exception& e) 
        {
            std::cout << "Invalid number: " << e.what() << "\n";
            return -1;
        }
    }

    void Server::updateClientWithExistingId(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client, uint64_t source_id) 
    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        if (clients_.find(client) != clients_.end()) 
        {
            std::cout << "Client is existing, update ID: " << source_id << "\n";
            clients_[client] = source_id;
        } 
        else 
        {
            // clients_.insert({client, source_id});
            clients_[client] = source_id;
        }
    }

    void Server::assignNewClientId(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client, uint64_t& source_id) 
    {
        {
            std::lock_guard<std::mutex> lock(client_id_mutex_);
            source_id = client_id_.empty() ? 0 : *client_id_.rbegin() + 1;
            client_id_.insert(source_id);
        }

        rat::Packet response;
        response.set_type(rat::Packet::STATIC_ID);
        response.set_packet_id("server_0_" + std::to_string(std::rand()));
        response.set_source_id("server_0");
        response.set_destination_id("client_" + std::to_string(source_id));
        response.set_encrypted(false);
        auto* chunked = response.mutable_chunked_data();
        chunked->set_data_id("STATIC_ID_" + std::to_string(std::rand()));
        chunked->set_sequence_number(0);
        chunked->set_total_chunks(1);
        chunked->set_payload(std::to_string(source_id));
        chunked->set_success(true);

        sendCommandToClient(client, response);
        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            std::cout << "Debug: " << source_id << std::endl;
            std::cout << "Debug set clients before insert: ";
            for(auto x: clients_) { std::cout << x.first << "-" << x.second << std::endl;}
            // clients_.insert({client, source_id});
            clients_[client] = source_id;
            std::cout << "Debug set clients after insert: ";
            for(auto x: clients_) {std::cout << x.first << "-" << x.second << std::endl;}
        }
    }

    void Server::saveClientIdsToFile(const uint64_t& source_id, const std::string& path) 
    {
        boost::asio::post(networkManager_.get_io_context(), [this, source_id, path]() 
        {
            std::lock_guard<std::mutex> lock(file_save_mutex_);
            std::ofstream file(path, std::ios::app);
            if (!file.is_open()) 
            {
                std::cout << "Cannot open file: " << path << "\n";
                return;
            }
            file << source_id << "\n";
            file.close();
        });
    }

} // namespace Rat
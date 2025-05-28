#include "NetworkManager.hpp"
#include <boost/asio.hpp>
#include <vector>
#include <cstring> // for memcpy
#include "Constants.hpp"

namespace Rat {
    NetworkManager::NetworkManager() {}
    NetworkManager::~NetworkManager() {}

    NetworkManager::IoContext& NetworkManager::get_io_context() {
        return io_context_;
    }

    void NetworkManager::send(std::shared_ptr<TcpSocket> socket, const rat::Packet& packet,
                               boost::function<void(const ErrorCode&)> callback) {
        // Serialize protobuf packet
        std::string serialized;
        if (!packet.SerializeToString(&serialized)) {
            callback({boost::asio::error::invalid_argument, boost::system::system_category()});
            return;
        }

        if (serialized.size() > MAX_BUFFER_SIZE) {
            callback({boost::asio::error::message_size, boost::system::system_category()});
            return;
        }

        // Prefix the serialized data with its length (uint32_t)
        uint32_t packet_size = static_cast<uint32_t>(serialized.size());
        auto full_buffer = std::make_shared<std::vector<char>>(sizeof(uint32_t) + packet_size);

        std::memcpy(full_buffer->data(), &packet_size, sizeof(uint32_t));
        std::memcpy(full_buffer->data() + sizeof(uint32_t), serialized.data(), packet_size);

        boost::asio::async_write(*socket, boost::asio::buffer(*full_buffer),
            [callback](const ErrorCode& ec, std::size_t) {
                callback(ec);
            });
    }

    void NetworkManager::receive(std::shared_ptr<TcpSocket> socket,
                                 boost::function<void(const rat::Packet&, const ErrorCode&)> callback) {
        // Step 1: Read packet size (4 bytes)
        auto size_buffer = std::make_shared<std::vector<char>>(sizeof(uint32_t));
        boost::asio::async_read(*socket, boost::asio::buffer(*size_buffer),
            [this, socket, size_buffer, callback](const ErrorCode& ec, std::size_t /*bytes_transferred*/) {
                if (ec) {
                    callback(rat::Packet(), ec);
                    return;
                }

                // Parse size
                uint32_t packet_size;
                std::memcpy(&packet_size, size_buffer->data(), sizeof(uint32_t));

                if (packet_size > MAX_BUFFER_SIZE) {
                    callback(rat::Packet(), {boost::asio::error::message_size, boost::system::system_category()});
                    return;
                }

                // Step 2: Read actual packet data
                auto data_buffer = std::make_shared<std::vector<char>>(packet_size);
                boost::asio::async_read(*socket, boost::asio::buffer(*data_buffer),
                    [data_buffer, callback](const ErrorCode& ec, std::size_t bytes_transferred) {
                        if (ec) {
                            callback(rat::Packet(), ec);
                            return;
                        }

                        if (bytes_transferred == 0) {
                            callback(rat::Packet(), {boost::asio::error::eof, boost::system::system_category()});
                            return;
                        }

                        rat::Packet packet;
                        
                        if (!packet.ParseFromArray(data_buffer->data(), static_cast<int>(bytes_transferred))) {
                            callback(rat::Packet(), {boost::asio::error::invalid_argument, boost::system::system_category()});
                            return;
                        }
                        // std::cout << "this this: " << packet.ByteSize() << std::endl;
                        callback(packet, {});
                    });
            });
    }
}

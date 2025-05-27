#include "NetworkManager.hpp"
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <vector>
#include <iomanip>

namespace Rat {

NetworkManager::NetworkManager() {}
NetworkManager::~NetworkManager() {}

NetworkManager::IoContext& NetworkManager::get_io_context() {
    return io_context_;
}

void NetworkManager::send(std::shared_ptr<TcpSocket> socket, const Command& command, boost::function<void(const ErrorCode&)> callback) {
    std::ostringstream oss;
    oss << static_cast<char>(command.type);
    oss << std::setw(10) << std::setfill('0') << command.payload_size;
    oss << command.payload;

    std::string buffer = oss.str();

    boost::asio::async_write(*socket, boost::asio::buffer(buffer),
        [callback](const boost::system::error_code& ec, std::size_t /*bytes_transferred*/) {
            callback(ec);
        });
}

void NetworkManager::receive(std::shared_ptr<TcpSocket> socket, boost::function<void(const Response&, const ErrorCode&)> callback) {
    auto header_buffer = std::make_shared<std::vector<char>>(11); // 1 byte type + 10 byte payload size

    boost::asio::async_read(*socket, boost::asio::buffer(*header_buffer),
        [this, socket, header_buffer, callback](const ErrorCode& ec, std::size_t /*bytes_transferred*/) {
            if (ec) {
                callback(Response(false, "", ec.message()), ec);
                return;
            }

            char type_char = (*header_buffer)[0];
            CommandType cmd_type = static_cast<CommandType>(type_char);

            std::string size_str(header_buffer->begin() + 1, header_buffer->end());
            size_t payload_size = 0;
            try {
                payload_size = std::stoul(size_str);
            } catch (const std::exception& e) {
                callback(Response(false, "", "Invalid payload size"),
                         boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                return;
            }

            if (payload_size > MAX_BUFFER_SIZE) {
                callback(Response(false, "", "Payload too large"),
                         boost::system::errc::make_error_code(boost::system::errc::value_too_large));
                return;
            }

            auto payload_buffer = std::make_shared<std::vector<char>>(payload_size);

            boost::asio::async_read(*socket, boost::asio::buffer(*payload_buffer),
                [cmd_type, payload_buffer, callback](const ErrorCode& ec2, std::size_t bytes_transferred) {
                    if (ec2) {
                        callback(Response(false, "", ec2.message()), ec2);
                        return;
                    }

                    std::string payload(payload_buffer->begin(), payload_buffer->begin() + bytes_transferred);
                    bool success = (cmd_type != CommandType::UNKNOWN);
                    callback(Response(success, payload), ec2);
                });
        });
}

// void NetworkManager::enqueue_command(const Command& command) {
//     boost::mutex::scoped_lock lock(queue_mutex_);
//     command_queue_.push(command);
// }

// bool NetworkManager::dequeue_command(Command& command) {
//     boost::mutex::scoped_lock lock(queue_mutex_);
//     if (command_queue_.empty()) {
//         return false;
//     }
//     command = command_queue_.front();
//     command_queue_.pop();
//     return true;
// }

} // namespace Rat
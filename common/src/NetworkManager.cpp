#include "NetworkManager.hpp"
#include <boost/asio.hpp>
#include <vector>
#include <cstring> // for memcpy
#include "Constants.hpp"

namespace Rat {
    NetworkManager::NetworkManager() : io_context_(), ssl_context_(boost::asio::ssl::context::tlsv12)
    {
        // setup default ssl context 
        ssl_context_.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3
        );
    }
    NetworkManager::~NetworkManager() {}

    NetworkManager::IoContext& NetworkManager::get_io_context() {
        return io_context_;
    }

    boost::asio::ssl::context& NetworkManager::get_ssl_context() {
       return ssl_context_;
    }
    
    void NetworkManager::setup_ssl_context(bool is_server, const std::string& cert_file, 
                                         const std::string& key_file, const std::string& verify_file) 
    {
        ssl_context_ = boost::asio::ssl::context(boost::asio::ssl::context::tlsv12);
        ssl_context_.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3
        );
        // Load self certificate and private key
        ssl_context_.use_certificate_chain_file(cert_file);
        ssl_context_.use_private_key_file(key_file, boost::asio::ssl::context::pem);

        // Load the opponent's certificate for verification
        ssl_context_.load_verify_file(verify_file);

        // Enable two-way authentication
        ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer | 
                                   boost::asio::ssl::verify_fail_if_no_peer_cert);

        // (Optional) Skip CA check in test environment
        ssl_context_.set_verify_callback([](bool preverified, boost::asio::ssl::verify_context&) {
            return true; // Skip CA check
        });
    }
    template<typename SocketType>
    void NetworkManager::send(std::shared_ptr<SocketType> socket, const rat::Packet& packet,
                             boost::function<void(const ErrorCode&)> callback)  
    {
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

    template<typename SocketType>
    void NetworkManager::receive(std::shared_ptr<SocketType> socket,
                                 boost::function<void(const rat::Packet&, const ErrorCode&)> callback) 
        {
        // Log socket type
        // if (std::is_same<SocketType, TcpSocket>::value) {
        //     std::cout << "Debug: Receiving on TcpSocket\n";
        // } else if (std::is_same<SocketType, SslSocket>::value) {
        //     std::cout << "Debug: Receiving on SslSocket\n";
        // } else {
        //     std::cout << "Debug: Unknown socket type: " << typeid(SocketType).name() << "\n";
        // }

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
                    std::cout << "Debug packet size: " << packet_size << std::endl;
                    callback(rat::Packet(), {boost::asio::error::message_size, boost::system::system_category()});
                    return;
                }
                // std::cout << "Debug packet size: " << packet_size << std::endl;
                // Step 2: Read actual packet data
                // std::cout << "Debug socket: " << socket << std::endl;
                // std::cout << "Debug packet_size" << packet_size << std::endl;
                auto data_buffer = std::make_shared<std::vector<char>>(packet_size);
                boost::asio::async_read(*socket, boost::asio::buffer(*data_buffer),
                    [data_buffer, callback](const ErrorCode& ec, std::size_t bytes_transferred) {
                        std::cout << "Debug read data: " << bytes_transferred << std::endl;
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
                            std::cout << "Debug packet type: " << packet.type() << std::endl;
                            std::cout << "Debug packet payload: " << packet.chunked_data().payload() << std::endl;
                            std::cout << "Debug payload size: " << sizeof(packet)  << std::endl;
                            callback(rat::Packet(), {boost::asio::error::invalid_argument, boost::system::system_category()});
                            return;
                        }
                        // std::cout << "Debug packet type: " << packet.type() << std::endl;
                        // std::cout << "Debug packet payload: " << packet.chunked_data().payload() << std::endl;
                        //  std::cout << "Debug payload size: " << sizeof(packet) << std::endl;
                        // std::cout << "Debug size packet: " << packet.ByteSize() << std::endl;
                        callback(packet, {});
                    });
            });
    }
    using IoContext = boost::asio::io_context;
    using TcpSocket = boost::asio::ip::tcp::socket;
    using SslSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ;
    using TcpAcceptor = boost::asio::ip::tcp::acceptor;
    using ErrorCode = boost::system::error_code ;
    // Explicit template instantiation
    template void NetworkManager::send<TcpSocket>(std::shared_ptr<TcpSocket>, const rat::Packet&, boost::function<void(const ErrorCode&)>);
    template void NetworkManager::send<SslSocket>(std::shared_ptr<SslSocket>, const rat::Packet&, boost::function<void(const ErrorCode&)>);
    template void NetworkManager::receive<TcpSocket>(std::shared_ptr<TcpSocket>, boost::function<void(const rat::Packet&, const ErrorCode&)>);
    template void NetworkManager::receive<SslSocket>(std::shared_ptr<SslSocket>, boost::function<void(const rat::Packet&, const ErrorCode&)>);
}

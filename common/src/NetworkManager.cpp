#include "NetworkManager.hpp"
#include <boost/asio.hpp>
#include <vector>
#include <cstring>
#include <iostream>
#include <openssl/x509.h>
#include <openssl/ssl.h>

namespace Rat 
{
    NetworkManager::NetworkManager() : io_context_(), ssl_context_(boost::asio::ssl::context::tls)
    {
        ssl_context_.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3
        );
    }

    NetworkManager::~NetworkManager() {}

    NetworkManager::IoContext& NetworkManager::get_io_context() 
    {
        return io_context_;
    }

    boost::asio::ssl::context& NetworkManager::get_ssl_context() 
    {
       return ssl_context_;
    }
    
    void NetworkManager::setup_ssl_context_client(const std::string& verify_file, const std::string& host) {
        ssl_context_ = boost::asio::ssl::context(boost::asio::ssl::context::tls);
        ssl_context_.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3
        );

        boost::system::error_code ec;
        ssl_context_.load_verify_file(verify_file, ec);
        if (ec) {
            std::cout << "[" << std::time(nullptr) << "] Failed to load CA certificate: " << ec.message() << std::endl;
            throw std::runtime_error("CA certificate loading failed");
        }

        SSL_CTX_set_info_callback(ssl_context_.native_handle(), [](const SSL* ssl, int where, int ret) {
            const char* str = SSL_state_string_long(ssl);
            std::cout << "[" << std::time(nullptr) << "] SSL Client: " << str << " (where: " << where << ", ret: " << ret << ")\n";
        });

        ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer);
        ssl_context_.set_verify_callback(
            [host](bool preverified, boost::asio::ssl::verify_context& ctx) {
                char subject_name[256];
                X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
                X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
                std::cout << "[" << std::time(nullptr) << "] Verifying server certificate: " << subject_name << std::endl;
                bool verified = boost::asio::ssl::rfc2818_verification(host)(preverified, ctx);
                if (!verified) {
                    std::cout << "[" << std::time(nullptr) << "] Certificate verification failed for host: " << host << std::endl;
                }
                return verified;
            });
    }

    void NetworkManager::setup_ssl_context_server(const std::string& cert_file, const std::string& key_file,
                                                const std::string& verify_file, const std::string& dh_file) {
        ssl_context_ = boost::asio::ssl::context(boost::asio::ssl::context::tls);
        ssl_context_.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3 |
            boost::asio::ssl::context::single_dh_use
        );

        boost::system::error_code ec;
        ssl_context_.use_certificate_chain_file(cert_file, ec);
        if (ec) {
            std::cout << "[" << std::time(nullptr) << "] Failed to load certificate: " << ec.message() << std::endl;
            throw std::runtime_error("Certificate loading failed");
        }
        ssl_context_.use_private_key_file(key_file, boost::asio::ssl::context::pem, ec);
        if (ec) {
            std::cout << "[" << std::time(nullptr) << "] Failed to load private key: " << ec.message() << std::endl;
            throw std::runtime_error("Private key loading failed");
        }
        ssl_context_.load_verify_file(verify_file, ec);
        if (ec) {
            std::cout << "[" << std::time(nullptr) << "] Failed to load CA certificate: " << ec.message() << std::endl;
            throw std::runtime_error("CA certificate loading failed");
        }
        if (!dh_file.empty()) {
            ssl_context_.use_tmp_dh_file(dh_file, ec);
            if (ec) {
                std::cout << "[" << std::time(nullptr) << "] Failed to load DH parameters: " << ec.message() << std::endl;
                throw std::runtime_error("DH parameters loading failed");
            }
        }

        SSL_CTX_set_info_callback(ssl_context_.native_handle(), [](const SSL* ssl, int where, int ret) {
            const char* str = SSL_state_string_long(ssl);
            std::cout << "[" << std::time(nullptr) << "] SSL Server: " << str << " (where: " << where << ", ret: " << ret << ")\n";
        });
        ssl_context_.set_verify_mode(boost::asio::ssl::verify_none);
    }

    template<typename SocketType>
    void NetworkManager::send(std::shared_ptr<SocketType> socket, const rat::Packet& packet,
                             boost::function<void(const ErrorCode&)> callback)  
    {
        // std::cout << "data send 0: " << std::endl;
        if (!socket || !socket->lowest_layer().is_open()) {
            std::cout << "[" << std::time(nullptr) << "] Socket not open for send\n";
            callback({boost::asio::error::not_connected, boost::system::system_category()});
            return;
        }

        std::string serialized;
        // std::cout << "data send 1: " << std::endl; 
        if (!packet.SerializeToString(&serialized)) {
            std::cout << "[" << std::time(nullptr) << "] Failed to serialize packet\n";
            callback({boost::asio::error::invalid_argument, boost::system::system_category()});
            return;
        }
        // std::cout << "data send 2: " << serialized << std::endl;
        if (serialized.size() > MAX_BUFFER_SIZE) {
            std::cout << "[" << std::time(nullptr) << "] Packet size exceeds maximum: " << serialized.size() << "\n";
            callback({boost::asio::error::message_size, boost::system::system_category()});
            return;
        }

        // uint32_t packet_size = static_cast<uint32_t>(serialized.size());
        // uint32_t packet_size = htonl(packet_size);
        // auto full_buffer = std::make_shared<std::vector<char>>(sizeof(uint32_t) + packet_size);
        // std::memcpy(full_buffer->data(), &packet_size, sizeof(uint32_t));
        // std::memcpy(full_buffer->data() + sizeof(uint32_t), serialized.data(), packet_size);

        uint32_t packet_size = static_cast<uint32_t>(serialized.size());
        uint32_t net_packet_size = htonl(packet_size);  // CHUYỂN endian

        auto full_buffer = std::make_shared<std::vector<char>>(sizeof(uint32_t) + packet_size);
        std::memcpy(full_buffer->data(), &net_packet_size, sizeof(uint32_t));  // Gửi bản đã chuyển endian
        std::memcpy(full_buffer->data() + sizeof(uint32_t), serialized.data(), packet_size);
        // std::cout << "data send 3: " << std::endl;
        boost::asio::async_write(*socket, boost::asio::buffer(*full_buffer),
            [callback](const ErrorCode& ec, std::size_t) {
                if (ec) {
                    std::cout << "[" << std::time(nullptr) << "] Send error: " << ec.message() << " (code: " << ec.value() << ")\n";
                }
                // else {std::cout << "send send " << std::endl;}
                callback(ec);
            });
    }

    template<typename SocketType>
    void NetworkManager::receive(std::shared_ptr<SocketType> socket,
                                 boost::function<void(const rat::Packet&, const ErrorCode&)> callback) 
    {
        if (!socket || !socket->lowest_layer().is_open()) {
            std::cout << "[" << std::time(nullptr) << "] Socket not open for receive\n";
            callback(rat::Packet(), {boost::asio::error::not_connected, boost::system::system_category()});
            return;
        }
        // std::cout << "Debug byte transfer size" << "gogogogogoggogogo" << std::endl;
        auto size_buffer = std::make_shared<std::vector<char>>(sizeof(uint32_t));
        boost::asio::async_read(*socket, boost::asio::buffer(*size_buffer),
            [this, socket, size_buffer, callback](const ErrorCode& ec, std::size_t /*bytes_transferred*/) {
                // std::cout << "Debug byte transfer size" << size_buffer->data() << std::endl;
                if (ec) {
                    std::cout << "[" << std::time(nullptr) << "] Receive size error: " << ec.message() << " (code: " << ec.value() << ")\n";
                    callback(rat::Packet(), ec);
                    return;
                }

                uint32_t packet_size;
                std::memcpy(&packet_size, size_buffer->data(), sizeof(uint32_t));
                packet_size = ntohl(packet_size);
                if (packet_size == 0 || packet_size > MAX_BUFFER_SIZE) {
                    std::cout << "[" << std::time(nullptr) << "] Invalid packet size: " << packet_size << "\n";
                    callback(rat::Packet(), {boost::asio::error::message_size, boost::system::system_category()});
                    return;
                }

                auto data_buffer = std::make_shared<std::vector<char>>(packet_size);
                boost::asio::async_read(*socket, boost::asio::buffer(*data_buffer),
                    [data_buffer, callback](const ErrorCode& ec, std::size_t bytes_transferred) {
                        if (ec) {
                            std::cout << "[" << std::time(nullptr) << "] Receive data error: " << ec.message() << " (code: " << ec.value() << ")\n";
                            callback(rat::Packet(), ec);
                            return;
                        }

                        rat::Packet packet;
                        if (!packet.ParseFromArray(data_buffer->data(), static_cast<int>(bytes_transferred))) {
                            std::cout << "[" << std::time(nullptr) << "] Failed to parse packet\n";
                            callback(rat::Packet(), {boost::asio::error::invalid_argument, boost::system::system_category()});
                            return;
                        }
                        callback(packet, {});
                    });
            });
    }

    using IoContext = boost::asio::io_context;
    using TcpSocket = boost::asio::ip::tcp::socket;
    using SslSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    using TcpAcceptor = boost::asio::ip::tcp::acceptor;
    using ErrorCode = boost::system::error_code;

    template void NetworkManager::send<SslSocket>(std::shared_ptr<SslSocket>, const rat::Packet&, boost::function<void(const ErrorCode&)>);
    template void NetworkManager::receive<SslSocket>(std::shared_ptr<SslSocket>, boost::function<void(const rat::Packet&, const ErrorCode&)>);
}
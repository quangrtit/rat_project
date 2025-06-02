#include "ServerSecurity.hpp"
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <fstream>
#include <iostream>
#include <memory>

namespace Rat 
{

    ServerSecurity::ServerSecurity(NetworkManager& networkManager)
        : networkManager_(networkManager) 
    {}

    ServerSecurity::~ServerSecurity() 
    {}

    bool ServerSecurity::signCsr(const std::string& csr, const std::string& ca_cert_file,
                                const std::string& ca_key_file, std::string& cert) 
    {
        std::cerr << "[Server] Signing CSR, size: " << csr.size() << "\n";

        BIO* bio = BIO_new_mem_buf(csr.c_str(), csr.size());
        X509_REQ* req = PEM_read_bio_X509_REQ(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (!req) 
        {
            std::cerr << "[Server] Failed to parse CSR\n";
            return false;
        }

        std::ifstream ca_cert_stream(ca_cert_file);
        if (!ca_cert_stream.is_open()) 
        {
            X509_REQ_free(req);
            std::cerr << "[Server] Cannot open CA certificate\n";
            return false;
        }
        std::string ca_cert_str((std::istreambuf_iterator<char>(ca_cert_stream)), std::istreambuf_iterator<char>());
        ca_cert_stream.close();

        BIO* ca_cert_bio = BIO_new_mem_buf(ca_cert_str.c_str(), ca_cert_str.size());
        X509* ca_cert = PEM_read_bio_X509(ca_cert_bio, nullptr, nullptr, nullptr);
        BIO_free(ca_cert_bio);
        if (!ca_cert) 
        {
            X509_REQ_free(req);
            std::cerr << "[Server] Failed to parse CA certificate\n";
            return false;
        }

        std::ifstream ca_key_stream(ca_key_file);
        if (!ca_key_stream.is_open()) 
        {
            X509_free(ca_cert);
            X509_REQ_free(req);
            std::cerr << "[Server] Cannot open CA key\n";
            return false;
        }
        std::string ca_key_str((std::istreambuf_iterator<char>(ca_key_stream)), std::istreambuf_iterator<char>());
        ca_key_stream.close();

        BIO* ca_key_bio = BIO_new_mem_buf(ca_key_str.c_str(), ca_key_str.size());
        EVP_PKEY* ca_key = PEM_read_bio_PrivateKey(ca_key_bio, nullptr, nullptr, nullptr);
        BIO_free(ca_key_bio);
        if (!ca_key) 
        {
            X509_free(ca_cert);
            X509_REQ_free(req);
            std::cerr << "[Server] Failed to parse CA key\n";
            return false;
        }

        X509* x509 = X509_new();
        if (!x509) 
        {
            EVP_PKEY_free(ca_key);
            X509_free(ca_cert);
            X509_REQ_free(req);
            std::cerr << "[Server] Failed to create new certificate\n";
            return false;
        }

        X509_set_version(x509, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(x509), rand());
        X509_gmtime_adj(X509_get_notBefore(x509), 0);
        X509_gmtime_adj(X509_get_notAfter(x509), 365 * 24 * 60 * 60);
        X509_set_pubkey(x509, X509_REQ_get_pubkey(req));
        X509_set_subject_name(x509, X509_REQ_get_subject_name(req));
        X509_set_issuer_name(x509, X509_get_subject_name(ca_cert));
        X509_sign(x509, ca_key, EVP_sha256());

        BIO* cert_bio = BIO_new(BIO_s_mem());
        PEM_write_bio_X509(cert_bio, x509);
        char* cert_data;
        long cert_len = BIO_get_mem_data(cert_bio, &cert_data);
        cert.assign(cert_data, cert_len);

        BIO_free(cert_bio);
        X509_free(x509);
        X509_free(ca_cert);
        EVP_PKEY_free(ca_key);
        X509_REQ_free(req);
        std::cerr << "[Server] CSR signed, certificate size: " << cert.size() << "\n";
        return true;
    }

    void ServerSecurity::authenticate(std::shared_ptr<TcpSocket> tcp_socket,
                                    const std::string& ca_cert_file,
                                    const std::string& ca_key_file,
                                    AuthCallback callback) 
    {
        std::cerr << "[Server] Starting authentication, ca_cert: " << ca_cert_file << "\n";

        if (!tcp_socket->is_open()) 
        {
            std::cerr << "[Server] Socket is closed\n";
            callback(false, boost::system::errc::make_error_code(boost::system::errc::not_connected));
            return;
        }

        auto timer = std::make_shared<boost::asio::steady_timer>(networkManager_.get_io_context());
        timer->expires_after(std::chrono::seconds(5));
        timer->async_wait([callback](const ErrorCode& ec) 
        {
            if (!ec) 
            {
                std::cerr << "[Server] Authentication timed out\n";
                callback(false, boost::system::errc::make_error_code(boost::system::errc::timed_out));
            }
        });
        // std::cout << "Debug interupt\n";
        // return;
        networkManager_.receive(tcp_socket, [this, tcp_socket, ca_cert_file, ca_key_file, timer, callback]
                                (const rat::Packet& packet, const ErrorCode& ec) 
        {
            timer->cancel();
            if (ec) 
            {
                std::cerr << "[Server] Receive error: " << ec.message() << "\n";
                tcp_socket->close();
                callback(false, ec);
                return;
            }

            std::cerr << "[Server] Received packet, type: " << packet.type() << "\n";

            if (packet.type() == rat::Packet::STATIC_ID) 
            {
                std::cerr << "[Server] Client sent STATIC_ID, skipping cert request\n";
                callback(true, ErrorCode());
                return;
            }

            if (packet.type() != rat::Packet::CERT_REQUEST) 
            {
                std::cerr << "[Server] Invalid packet type: " << packet.type() << "\n";
                tcp_socket->close();
                callback(false, boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                return;
            }

            const std::string& csr = packet.chunked_data().payload();
            std::cerr << "[Server] Received CERT_REQUEST, CSR size: " << csr.size() << "\n";

            std::string cert;
            if (!signCsr(csr, ca_cert_file, ca_key_file, cert)) 
            {
                std::cerr << "[Server] Failed to sign CSR\n";
                tcp_socket->close();
                callback(false, boost::system::errc::make_error_code(boost::system::errc::operation_canceled));
                return;
            }

            rat::Packet response;
            response.set_type(rat::Packet::CERT_RESPONSE);
            response.set_packet_id("server_" + std::to_string(std::rand()));
            response.set_source_id("server_0");
            response.set_destination_id("client_-1");
            response.mutable_chunked_data()->set_payload(cert);

            std::cerr << "[Server] Sending CERT_RESPONSE, payload_size: " << cert.size() << "\n";
            networkManager_.send(tcp_socket, response, [tcp_socket, callback](const ErrorCode& ec) 
            {
                if (ec) 
                {
                    std::cerr << "[Server] Failed to send CERT_RESPONSE: " << ec.message() << "\n";
                    tcp_socket->close();
                    callback(false, ec);
                    return;
                }
                std::cerr << "[Server] Sent CERT_RESPONSE\n";
                callback(true, ErrorCode());
            });
        });
    }

    std::shared_ptr<ServerSecurity::SslSocket> ServerSecurity::createSslSocket(std::shared_ptr<TcpSocket> tcp_socket,
                                                                            boost::asio::ssl::context& ssl_context) 
    {
        auto ssl_socket = std::make_shared<SslSocket>(tcp_socket->get_executor(), ssl_context);
        ssl_socket->lowest_layer() = std::move(*tcp_socket);
        return ssl_socket;
    }

} // namespace Rat
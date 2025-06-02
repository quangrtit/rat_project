#include "ClientSecurity.hpp"
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <unistd.h>

namespace Rat 
{

    ClientSecurity::ClientSecurity(NetworkManager& networkManager)
        : networkManager_(networkManager) 
        {}

    ClientSecurity::~ClientSecurity() 
    {}

    bool ClientSecurity::generateClientCertificate(const std::string& key_file, const std::string& csr_file) 
    {
        EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        if (!pctx || EVP_PKEY_keygen_init(pctx) <= 0 ||
            EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0) 
        {
            if (pctx) EVP_PKEY_CTX_free(pctx);
            std::cerr << "[Client] Failed to init keygen\n";
            return false;
        }

        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(pctx, &pkey) <= 0) 
        {
            EVP_PKEY_CTX_free(pctx);
            std::cerr << "[Client] Failed to generate key\n";
            return false;
        }
        EVP_PKEY_CTX_free(pctx);

        X509_REQ* req = X509_REQ_new();
        if (!req || X509_REQ_set_version(req, 0) != 1) 
        {
            EVP_PKEY_free(pkey);
            if (req) X509_REQ_free(req);
            std::cerr << "[Client] Failed to create CSR\n";
            return false;
        }

        X509_NAME* name = X509_REQ_get_subject_name(req);
        if (!name || !X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                                                (unsigned char*)"client", -1, -1, 0)) 
        {
            EVP_PKEY_free(pkey);
            X509_REQ_free(req);
            std::cerr << "[Client] Failed to set subject\n";
            return false;
        }

        if (X509_REQ_set_pubkey(req, pkey) != 1 || X509_REQ_sign(req, pkey, EVP_sha256()) <= 0) 
        {
            EVP_PKEY_free(pkey);
            X509_REQ_free(req);
            std::cerr << "[Client] Failed to sign CSR\n";
            return false;
        }

        FILE* key_fp = fopen(key_file.c_str(), "wb");
        if (!key_fp || PEM_write_PrivateKey(key_fp, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1) 
        {
            if (key_fp) fclose(key_fp);
            EVP_PKEY_free(pkey);
            X509_REQ_free(req);
            std::cerr << "[Client] Failed to write key\n";
            return false;
        }
        fclose(key_fp);

        FILE* csr_fp = fopen(csr_file.c_str(), "wb");
        if (!csr_fp || PEM_write_X509_REQ(csr_fp, req) != 1) 
        {
            if (csr_fp) fclose(csr_fp);
            EVP_PKEY_free(pkey);
            X509_REQ_free(req);
            std::cerr << "[Client] Failed to write CSR\n";
            return false;
        }
        fclose(csr_fp);

        EVP_PKEY_free(pkey);
        X509_REQ_free(req);
        std::cerr << "[Client] Generated CSR and key\n";
        return true;
    }

    bool ClientSecurity::validateCertificate(const std::string& cert_file, const std::string& key_file) 
    {
        std::cerr << "[Client] Validating certificate: " << cert_file << ", key: " << key_file << "\n";

        std::ifstream cert_stream(cert_file);
        if (!cert_stream.is_open()) 
        {
            std::cerr << "[Client] Cannot open cert: " << cert_file << "\n";
            return false;
        }
        std::string cert_str((std::istreambuf_iterator<char>(cert_stream)), std::istreambuf_iterator<char>());
        cert_stream.close();

        BIO* cert_bio = BIO_new_mem_buf(cert_str.c_str(), cert_str.size());
        X509* cert = PEM_read_bio_X509(cert_bio, nullptr, nullptr, nullptr);
        BIO_free(cert_bio);
        if (!cert) 
        {
            std::cerr << "[Client] Failed to parse cert\n";
            return false;
        }

        std::ifstream key_stream(key_file);
        if (!key_stream.is_open()) 
        {
            X509_free(cert);
            std::cerr << "[Client] Cannot open key: " << key_file << "\n";
            return false;
        }
        std::string key_str((std::istreambuf_iterator<char>(key_stream)), std::istreambuf_iterator<char>());
        key_stream.close();

        BIO* key_bio = BIO_new_mem_buf(key_str.c_str(), key_str.size());
        EVP_PKEY* pkey = PEM_read_bio_PrivateKey(key_bio, nullptr, nullptr, nullptr);
        BIO_free(key_bio);
        if (!pkey) 
        {
            X509_free(cert);
            std::cerr << "[Client] Failed to parse key\n";
            return false;
        }

        EVP_PKEY* cert_pkey = X509_get_pubkey(cert);
        bool valid = EVP_PKEY_eq(cert_pkey, pkey) == 1;
        EVP_PKEY_free(cert_pkey);
        EVP_PKEY_free(pkey);
        X509_free(cert);
        std::cerr << "[Client] Cert validation: " << (valid ? "valid" : "invalid") << "\n";
        return valid;
    }

    bool ClientSecurity::validateCsr(const std::string& csr_file) 
    {
        std::cerr << "[Client] Validating CSR: " << csr_file << "\n";

        std::ifstream csr_stream(csr_file);
        if (!csr_stream.is_open()) 
        {
            std::cerr << "[Client] Cannot open CSR file: " << csr_file << "\n";
            return false;
        }
        std::string csr_str((std::istreambuf_iterator<char>(csr_stream)), std::istreambuf_iterator<char>());
        csr_stream.close();

        BIO* csr_bio = BIO_new_mem_buf(csr_str.c_str(), csr_str.size());
        X509_REQ* req = PEM_read_bio_X509_REQ(csr_bio, nullptr, nullptr, nullptr);
        BIO_free(csr_bio);
        if (!req) 
        {
            std::cerr << "[Client] Failed to parse CSR\n";
            return false;
        }

        EVP_PKEY* pubkey = X509_REQ_get_pubkey(req);
        if (!pubkey) 
        {
            X509_REQ_free(req);
            std::cerr << "[Client] Failed to get public key from CSR\n";
            return false;
        }

        bool valid = X509_REQ_verify(req, pubkey) == 1;
        EVP_PKEY_free(pubkey);
        X509_REQ_free(req);
        std::cerr << "[Client] CSR validation: " << (valid ? "valid" : "invalid") << "\n";
        return valid;
    }

    void ClientSecurity::authenticate(std::shared_ptr<TcpSocket> tcp_socket,
                                    const std::string& ca_cert_file,
                                    AuthCallback callback) 
    {
        std::cerr << "[Client] Starting authentication, ca_cert: " << ca_cert_file << "\n";
        
        // There is currently an error for future certificate validations that cannot be fixed. It will be fixed later.
        // Check existing certificate
        // if (access("./client.crt", F_OK) == 0 && access("./client.key", F_OK) == 0 &&
        //     validateCertificate("./client.crt", "./client.key")) {
        //     std::cerr << "[Client] Using existing valid certificate\n";
        //     callback(true, ErrorCode());
        //     return;
        // }

        // Remove old files
        unlink("./client.crt");
        unlink("./client.key");
        unlink("./client.csr");

        auto timer = std::make_shared<boost::asio::steady_timer>(networkManager_.get_io_context());

        // Generate new client certificate and CSR
        if (!generateClientCertificate("./client.key", "./client.csr")) 
        {
            std::cerr << "[Client] Failed to generate client certificate\n";
            callback(false, boost::system::errc::make_error_code(boost::system::errc::operation_canceled));
            return;
        }

        std::ifstream csr_stream("./client.csr");
        if (!csr_stream.is_open()) 
        {
            std::cerr << "[Client] Cannot open CSR file\n";
            callback(false, boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory));
            return;
        }
        std::string csr((std::istreambuf_iterator<char>(csr_stream)), std::istreambuf_iterator<char>());
        csr_stream.close();
        std::cerr << "[Client] CSR size: " << csr.size() << "\n";

        rat::Packet packet;
        packet.set_type(rat::Packet::CERT_REQUEST);
        packet.set_packet_id("client_" + std::to_string(std::rand()));
        packet.set_source_id("client_-1");
        packet.set_destination_id("server_0");
        packet.mutable_chunked_data()->set_payload(csr);
        packet.mutable_chunked_data()->set_total_chunks(1);

        std::cerr << "[Client] Sending CERT_REQUEST, packet_id: " << packet.packet_id() << "\n";
        networkManager_.send(tcp_socket, packet, [this, tcp_socket, timer, callback](const ErrorCode& ec) 
        {
            if (ec) 
            {
                std::cerr << "[Client] Failed to send CERT_REQUEST: " << ec.message() << "\n";
                tcp_socket->close();
                callback(false, ec);
                return;
            }

            std::cerr << "[Client] CERT_REQUEST sent, waiting for CERT_RESPONSE\n";

            timer->expires_after(std::chrono::seconds(5));
            timer->async_wait([callback](const ErrorCode& ec) 
            {
                if (!ec) 
                {
                    std::cerr << "[Client] Authentication timed out\n";
                    callback(false, boost::system::errc::make_error_code(boost::system::errc::timed_out));
                }
            });

            networkManager_.receive(tcp_socket, [tcp_socket, timer, callback](const rat::Packet& response, const ErrorCode& ec) 
            {
                timer->cancel();
                if (ec) 
                {
                    std::cerr << "[Client] Receive error: " << ec.message() << "\n";
                    tcp_socket->close();
                    callback(false, ec);
                    return;
                }
                if (response.type() != rat::Packet::CERT_RESPONSE) 
                {
                    std::cerr << "[Client] Invalid CERT_RESPONSE: type=" << response.type() << "\n";
                    tcp_socket->close();
                    callback(false, boost::system::errc::make_error_code(boost::system::errc::protocol_error));
                    return;
                }

                std::cerr << "[Client] Received CERT_RESPONSE, payload_size: " << response.chunked_data().payload().size() << "\n";

                std::ofstream cert_stream("./client.crt");
                if (!cert_stream.is_open()) 
                {
                    std::cerr << "[Client] Cannot write client certificate\n";
                    tcp_socket->close();
                    callback(false, boost::system::errc::make_error_code(boost::system::errc::permission_denied));
                    return;
                }
                const std::string& payload = response.chunked_data().payload();
                cert_stream.write(payload.data(), static_cast<std::streamsize>(payload.size()));
                cert_stream.close();
                std::cerr << "[Client] Saved client certificate\n";
                callback(true, ErrorCode());
            });
        });
    }

    std::shared_ptr<ClientSecurity::SslSocket> ClientSecurity::createSslSocket(std::shared_ptr<TcpSocket> tcp_socket,
                                                                            boost::asio::ssl::context& ssl_context) 
    {
        auto ssl_socket = std::make_shared<SslSocket>(tcp_socket->get_executor(), ssl_context);
        ssl_socket->lowest_layer() = std::move(*tcp_socket);
        return ssl_socket;
    }

} // namespace Rat
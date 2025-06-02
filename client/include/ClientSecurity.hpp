#ifndef RAT_CLIENT_SECURITY_HPP
#define RAT_CLIENT_SECURITY_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <memory>
#include <functional>
#include "NetworkManager.hpp"
#include "Packet.pb.h"

namespace Rat 
{

    class ClientSecurity 
    {
    public:
        using TcpSocket = boost::asio::ip::tcp::socket;
        using SslSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
        using ErrorCode = boost::system::error_code;
        using AuthCallback = std::function<void(bool success, const ErrorCode& ec)>;

        ClientSecurity(NetworkManager& networkManager);
        ~ClientSecurity();

        void authenticate(std::shared_ptr<TcpSocket> tcp_socket,
                        const std::string& ca_cert_file,
                        AuthCallback callback);

        std::shared_ptr<SslSocket> createSslSocket(std::shared_ptr<TcpSocket> tcp_socket,
                                                boost::asio::ssl::context& ssl_context);

    private:
        bool generateClientCertificate(const std::string& key_file, const std::string& csr_file);
        bool validateCertificate(const std::string& cert_file, const std::string& key_file);
        bool validateCsr(const std::string& csr_file);

        NetworkManager& networkManager_;
    };

} // namespace Rat

#endif // RAT_CLIENT_SECURITY_HPP
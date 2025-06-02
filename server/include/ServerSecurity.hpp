#ifndef RAT_SERVER_SECURITY_HPP
#define RAT_SERVER_SECURITY_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <memory>
#include <functional>
#include "NetworkManager.hpp"
#include "Packet.pb.h"

namespace Rat 
{

    class ServerSecurity 
    {
    public:
        using TcpSocket = boost::asio::ip::tcp::socket;
        using SslSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
        using ErrorCode = boost::system::error_code;
        using AuthCallback = std::function<void(bool success, const ErrorCode& ec)>;

        ServerSecurity(NetworkManager& networkManager);
        ~ServerSecurity();

        void authenticate(std::shared_ptr<TcpSocket> tcp_socket,
                        const std::string& ca_cert_file,
                        const std::string& ca_key_file,
                        AuthCallback callback);

        std::shared_ptr<SslSocket> createSslSocket(std::shared_ptr<TcpSocket> tcp_socket,
                                                boost::asio::ssl::context& ssl_context);

    private:
        bool signCsr(const std::string& csr, const std::string& ca_cert_file,
                    const std::string& ca_key_file, std::string& cert);

        NetworkManager& networkManager_;
    };

} // namespace Rat

#endif // RAT_SERVER_SECURITY_HPP
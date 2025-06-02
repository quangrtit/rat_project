#ifndef RAT_NETWORK_MANAGER_HPP
#define RAT_NETWORK_MANAGER_HPP

#include <boost/asio.hpp>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <memory>
#include <boost/asio/ssl.hpp>
#include "Constants.hpp"
#include "Packet.pb.h"

namespace Rat 
{
    class NetworkManager 
    {
    public:
        typedef boost::asio::io_context IoContext;
        typedef boost::asio::ip::tcp::socket TcpSocket;
        typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> SslSocket;
        typedef boost::asio::ip::tcp::acceptor TcpAcceptor;
        typedef boost::system::error_code ErrorCode;

        NetworkManager();
        ~NetworkManager();
        IoContext& get_io_context();
        boost::asio::ssl::context& get_ssl_context();
        void setup_ssl_context(bool is_sever, const std::string& cert_file, 
                            const std::string& key_file, const std::string& verify_file);
                            
        template<typename SocketType>
        void send(std::shared_ptr<SocketType> socket, const rat::Packet& packet,
                  boost::function<void(const ErrorCode&)> callback);

        template<typename SocketType>
        void receive(std::shared_ptr<SocketType> socket,
                     boost::function<void(const rat::Packet&, const ErrorCode&)> callback);
    
    private:
        IoContext io_context_;
        boost::asio::ssl::context ssl_context_;
        // std::queue<Command> command_queue_;
        // boost::mutex queue_mutex_;
    };
};

#endif // RAT_NETWORK_MANAGER_HPP
//RECEIVE AND SEND FORMAT: Packet.proto
// reference: https://www.boost.org/doc/libs/latest/doc/html/boost_asio/tutorial.html
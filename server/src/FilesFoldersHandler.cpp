// FilesFoldersHandler.cpp
#include "FilesFoldersHandler.hpp"
#include "Packet.pb.h"
#include <iostream>

namespace Rat
{
    FilesFoldersHandler::FilesFoldersHandler(std::weak_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket,
                                   NetworkManager& networkManager,
                                   uint64_t client_id,
                                   const std::string& data_id,
                                   std::function<void(const std::string&)> on_complete)
        : DataHandler(socket, networkManager, client_id, data_id, on_complete)
    {
    }

    int FilesFoldersHandler::getPacketType() const
    {
        return rat::Packet::LIST_FILES_FOLDERS;
    }

    void FilesFoldersHandler::startReceiving(const rat::Packet& initial_packet)
    {
        // Call the base class implementation
        DataHandler::startReceiving(initial_packet);
    }
} // namespace Rat
// ProcessHandler.hpp
#ifndef PROCESS_HANDLER_HPP
#define PROCESS_HANDLER_HPP

#include "DataHandler.hpp"
#include <memory>
#include <functional>
#include <string>

namespace Rat
{
    class ProcessHandler : public DataHandler
    {
    public:
        ProcessHandler(std::weak_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket,
                       NetworkManager& networkManager,
                       uint64_t client_id,
                       const std::string& data_id,
                       std::function<void(const std::string&)> on_complete);

        void startReceiving(const rat::Packet& initial_packet) override;

    protected:
        int getPacketType() const override;
    };

} // namespace Rat

#endif // PROCESS_HANDLER_HPP
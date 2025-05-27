#ifndef RAT_CONSTANTS_HPP
#define RAT_CONSTANTS_HPP

#include <cstdint>
#include <string>
#include <stdexcept>

namespace Rat
{
    const uint16_t DEFAULT_PORT = 8080;
    const size_t MAX_BUFFER_SIZE = 8192;
    const size_t MAX_CLIENT = 1000;
    const std::string KEY_AES = "";
    const std::string LOG_DIR = "var/log/rat";


    enum CommandType {
        LIST_FILES,
        READ_FILE,
        TRANSFER_FILE,
        LIST_PROCESSES,
        KILL_PROCESS,
        UNKNOWN
    };

    struct Command 
    {
        CommandType type;
        std::string payload;
        size_t payload_size;
        
        Command() : type(CommandType::UNKNOWN), payload_size(0) {}
        Command(CommandType t, const std::string& p) : type(t), payload(p), payload_size(p.size())
        {
            if (payload_size > MAX_BUFFER_SIZE) 
            {
                throw std::runtime_error("Payload exceeds max buffer size");
            }
        }
    };

    struct Response 
    {
        bool success;
        std::string data;
        std::string error_message;
        size_t data_size;

        Response() : success(false), data_size(0) {}
        Response(bool s, const std::string& d, const std::string& e = "") : success(s), data(d), error_message(e), data_size(d.size()) 
        {
            if(data_size > MAX_BUFFER_SIZE)
            {
                throw std::runtime_error("Response data exceeds max buffer size");
            }
        }
    };
}; // namespace rat

#endif // RAT_CONSTANTS_HPP
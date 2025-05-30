#ifndef RAT_CONSTANTS_HPP
#define RAT_CONSTANTS_HPP

#include <cstdint>
#include <string>
#include <stdexcept>

namespace Rat
{
    const uint16_t DEFAULT_PORT = 8080;
    const size_t MAX_BUFFER_SIZE = 64 * 1024;
    const size_t MAX_CLIENT = 1000;
    const std::string KEY_AES = "";
    const std::string LOG_DIR = "var/log/rat";


    enum CommandType {
        LIST_FILES,
        READ_FILE,
        TRANSFER_FILE,
        LIST_PROCESSES,
        KILL_PROCESS,
        IDENTIFY,
        ACK,
        ERROR,
        UNKNOWN
    };
   
}; // namespace rat

#endif // RAT_CONSTANTS_HPP
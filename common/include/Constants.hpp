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
        UNKNOWN, //0;
        LIST_FILES, //1;
        READ_FILE, //2;
        TRANSFER_FILE, //3;
        LIST_PROCESSES, //4;
        KILL_PROCESS, //5;
        CERT_REQUEST, //6;
        CERT_RESPONSE, //7;
        AUTH_CONFIRM, //8;
        ERROR, //9;
        COMMAND, //10;
        STATIC_ID //11;
    };
   
}; // namespace rat

#endif // RAT_CONSTANTS_HPP
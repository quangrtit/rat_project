#ifndef UTILS_HPP
#define UTILS_HPP 

#include <vector> 
#include <iostream>
#include <string>
#include <iomanip>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <unordered_map>
// #include "Packet.pb.h"

namespace Rat 
{
    class Utils  
    {
    public:
        
        Utils();

        ~Utils();

        static std::vector<std::string> handleCommand(const std::string& cmd);

        static bool checkObjectAndGetId(const std::string& object, std::string& object_id);

        // static bool checkArgument(const std::string& argument);
        
        static std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> getSocketFromId(
            const std::unordered_map<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>, uint64_t> &clients, 
            uint64_t& object_id);

        static std::string sanitizeFileName(const std::string& name);
        
        static std::string getFileName(const std::string& path);
    };
}


#endif // UTILS_HPP
#include "Utils.hpp"
#include "ServerGUI.hpp"
#include <sstream>
#include <iomanip>
#include <random>

namespace Rat
{
    Utils::Utils() {}

    Utils::~Utils() {}

    std::string Utils::md5HashString(const std::string &input)
    {
        unsigned char md_value[EVP_MAX_MD_SIZE];
        unsigned int md_len = 0;
        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(mdctx, EVP_md5(), nullptr);
        EVP_DigestUpdate(mdctx, input.data(), input.size());
        EVP_DigestFinal_ex(mdctx, md_value, &md_len);
        EVP_MD_CTX_free(mdctx);
        std::stringstream ss;
        for (unsigned int i = 0; i < md_len; ++i)
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)md_value[i];
        return ss.str();
    }

    std::string Utils::generateRandomData(size_t length)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(33, 126);
        std::string data;
        for (size_t i = 0; i < length; ++i)
        {
            data += static_cast<char>(dis(gen));
        }
        return data;
    }

    std::string Utils::getCurrentTimeString()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        std::time_t t = system_clock::to_time_t(now);
        std::tm local_time;
        localtime_r(&t, &local_time);
        std::ostringstream oss;
        oss << std::put_time(&local_time, "%H%M%S%d%m%Y");
        oss << std::setw(3) << std::setfill('0') << ms.count();
        return oss.str();
    }
    std::vector<std::string> Utils::handleCommand(const std::string &cmd)
    {
        std::stringstream ss(cmd);
        std::string command;
        std::vector<std::string> list_commands;

        while (std::getline(ss, command, ' '))
        {
            if (!command.empty())
            {
                list_commands.push_back(command);
            }
        }
        if (list_commands.size() == 2)
        {
            std::string command = list_commands[0];
            std::string object = list_commands[1];
            if (!checkObjectAndGetId(object, list_commands[1]))
            {
                std::cout << "Server Command Error!\n";
                return std::vector<std::string>();
            }
        }
        else if (list_commands.size() == 3)
        {
            std::string command = list_commands[0];
            std::string object = list_commands[1];
            std::string argument = list_commands[2];
            if (!checkObjectAndGetId(object, list_commands[1]))
            {
                std::cout << "Server Command Error!\n";
                return std::vector<std::string>();
            }
        }
        else if (list_commands.size() == 1)
        {
            if (list_commands[0] == "exit")
            {
                std::cout << "Server Exit!\n";
                return list_commands;
            }
            else if (list_commands[0] == "help")
            {
                std::cout << "Server Help!\n";
                // ServerGUI::displayMenu();
            }
            else if (list_commands[0] == "list_clients")
            {
                std::cout << "Server List Clients!\n";
                // ServerGUI::displayClients(clients_);
            }
            else
            {
                return std::vector<std::string>();
            }
        }
        return list_commands;
    }
    // check format ip/id
    bool Utils::checkObjectAndGetId(const std::string &object, std::string &object_id)
    {
        // Check for whitespace throughout string
        if (object.find(' ') != std::string::npos)
            return false;

        size_t slashPos = object.find('/');
        if (slashPos == std::string::npos)
            return false;

        std::string ip = object.substr(0, slashPos);
        std::string id = object.substr(slashPos + 1);
        object_id = id;
        auto isValidIPv4 = [](const std::string &ip) -> bool
        {
            std::stringstream ss(ip);
            std::string token;
            int count = 0;

            while (std::getline(ss, token, '.'))
            {
                if (++count > 4)
                    return false; // IP has only 4 parts

                // Do not leave blank, do not contain letters, do not exceed 255
                if (token.empty())
                    return false;
                for (char c : token)
                {
                    if (!std::isdigit(c))
                        return false;
                }
                int num = std::stoi(token);
                if (num < 0 || num > 255)
                    return false;
            }
            return count == 4;
        };
        auto isValidId = [](const std::string &id) -> bool
        {
            if (id.empty())
                return false;
            for (char c : id)
            {
                if (!std::isdigit(c))
                    return false;
            }
            return true;
        };
        return isValidIPv4(ip) && isValidId(id);
    }
    // check format argument
    // bool Utils::checkArgument(const std::string& argument)
    // {
    //     return true;
    // }

    std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> Utils::getSocketFromId(
        const std::unordered_map<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>, uint64_t> &clients,
        uint64_t &object_id)
    {
        for (auto &socket_id : clients)
        {

            if (socket_id.second == object_id)
            {
                return socket_id.first;
            }
        }
        std::cout << "Server Error: No Find Socket Id!\n";
        return nullptr;
    }
    std::string Utils::sanitizeFileName(const std::string &name)
    {
        std::string sanitized = name;
        // Thay thế ký tự không hợp lệ bằng '_'
        std::replace(sanitized.begin(), sanitized.end(), ':', '_');
        std::replace(sanitized.begin(), sanitized.end(), ' ', '_');
        std::replace(sanitized.begin(), sanitized.end(), '/', '_');
        std::replace(sanitized.begin(), sanitized.end(), '\\', '_');
        return sanitized;
    }
    std::string Utils::getFileName(const std::string &path)
    {
        size_t pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }
}
#ifndef SERVER_GUI_HPP
#define SERVER_GUI_HPP 

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include <map>
#include <memory>
#include <mutex>
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>

// ANSI escape codes for colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define BOLD    "\033[1m"

namespace Rat 
{
    class ServerGUI 
    {
    public:
        using SslSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

        ServerGUI();

        ~ServerGUI();

        // Show list clients
        static void displayClients(const std::unordered_map<std::shared_ptr<SslSocket>, uint64_t>& clients);

        // Show menu command 
        static void displayMenu();

        // Show response data from client 
        static void displayResult(uint64_t client_id, const std::string& ip, const std::string& command, const std::string& result);

        // Show error
        static void displayError(const std::string& error);

        // Display file transfer progress
        static void displayProgress(uint64_t client_id, const std::string& ip, const std::string& filename, 
                                   uint64_t sequence_number, uint64_t total_chunks);

    private:
        // Structure to store progress information for each client
        struct ProgressInfo {
            std::string ip;
            std::string filename;
            uint64_t sequence_number;
            uint64_t total_chunks;
        };

        // Map to store progress for each client
        static std::map<uint64_t, ProgressInfo> progress_map_;
        // Mutex to protect console output and progress_map
        static std::mutex console_mutex_;
    };
}

#endif // SERVER_GUI_HPP
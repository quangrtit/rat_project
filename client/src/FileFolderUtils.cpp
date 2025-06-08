// FileFolderUtils.cpp
#include "FileFolderUtils.hpp"
#include <dirent.h>
#include <string.h>
#include <sstream>
#include <iostream>

namespace Rat
{
    std::string FileFolderUtils::listFilesFolders(const std::string& path_and_level)
    {
        std::cout << "Debug start get list file and folder: " << path_and_level << std::endl;
        auto parsed = parsePathAndLevel(path_and_level);
        std::string path = std::get<0>(parsed);
        int level = std::get<1>(parsed);
        bool include_hidden = std::get<2>(parsed);
        std::cout << "Parsed path: " << path << ", level: " << level << ", include_hidden: " << include_hidden << std::endl;

        // check exis path ???  
        DIR* dir = opendir(path.c_str());
        if (!dir)
        {
            std::cerr << "Cannot open directory: " << path << "\n";
            return "";
        }
        closedir(dir);

        std::vector<std::string> items;
        traverseDirectory(path, 0, level, items, include_hidden);
        return formatFileFolderList(items, 0);
    }

    void FileFolderUtils::traverseDirectory(const std::string& path, int current_level, int max_level,
                                           std::vector<std::string>& result, bool include_hidden)
    {
        DIR* dir = opendir(path.c_str());
        if (!dir)
        {
            std::cerr << "Cannot open directory: " << path << "\n";
            return;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string name = entry->d_name;
            // Bỏ qua . và .., và các file/folder ẩn nếu include_hidden là false
            if ((name[0] == '.' && !include_hidden) || strcmp(name.c_str(), ".") == 0 || strcmp(name.c_str(), "..") == 0)
                continue;

            result.push_back(std::to_string(current_level) + ":" + name);

            if (current_level < max_level && entry->d_type == DT_DIR)
            {
                std::string new_path = path + "/" + name;
                traverseDirectory(new_path, current_level + 1, max_level, result, include_hidden);
            }
        }
        closedir(dir);
    }

    std::string FileFolderUtils::formatFileFolderList(const std::vector<std::string>& items, uint64_t base_level)
    {
        std::ostringstream oss;
        for (const auto& item : items)
        {
            oss << item << "\n";
        }
        return oss.str();
    }

    std::tuple<std::string, int, bool> FileFolderUtils::parsePathAndLevel(const std::string& path_and_level)
    {
        std::string path, levelStr, hideStr;
        int level = 0;
        bool include_hidden = false;

        std::cout << "Debug 1" << std::endl;
        size_t lastUnderscorePos = path_and_level.find_last_of('|');
        if (lastUnderscorePos == std::string::npos)
        {
            return {".", 0, false}; 
        }

        
        hideStr = path_and_level.substr(lastUnderscorePos + 1);
        include_hidden = (hideStr == "1");

       
        size_t firstUnderscorePos = path_and_level.find_first_of('|');
        if (firstUnderscorePos != std::string::npos && firstUnderscorePos < lastUnderscorePos)
        {
            levelStr = path_and_level.substr(firstUnderscorePos + 1, lastUnderscorePos - firstUnderscorePos - 1);
            level = std::stoi(levelStr);
        }

    
        path = path_and_level.substr(0, firstUnderscorePos);

        return {path, level, include_hidden};
    }
} // namespace Rat
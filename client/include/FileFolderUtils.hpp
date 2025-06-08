// FileFolderUtils.hpp
#ifndef FILE_FOLDER_UTILS_HPP
#define FILE_FOLDER_UTILS_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <tuple>
namespace Rat
{
    class FileFolderUtils
    {
    public:
        static std::string listFilesFolders(const std::string& path_and_level);
        static void traverseDirectory(const std::string& path, int current_level, int max_level,
                                     std::vector<std::string>& result, bool include_hidden);
        static std::string formatFileFolderList(const std::vector<std::string>& items, uint64_t base_level);

    private:
        static std::tuple<std::string, int, bool> parsePathAndLevel(const std::string& path_and_level);
    };
} // namespace Rat

#endif // FILE_FOLDER_UTILS_HPP
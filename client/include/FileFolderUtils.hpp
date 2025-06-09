// FileFolderUtils.hpp
#ifndef FILE_FOLDER_UTILS_HPP
#define FILE_FOLDER_UTILS_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <tuple>
#include <optional>
namespace Rat
{
    class FileFolderUtils 
    {
    public:
        static std::string listFilesFolders(const std::string& input, std::string& response);
    private:
        static void traverseDirectory(const std::string& path, uint32_t current_level, uint32_t max_level,
                                      std::vector<std::string>& result, bool include_hidden);
        static std::string formatFileFolderList(const std::vector<std::string>& items);
        static std::optional<std::tuple<std::string, uint32_t, bool>> parsePathAndLevel(const std::string& input);
    };
} // namespace Rat

#endif // FILE_FOLDER_UTILS_HPP
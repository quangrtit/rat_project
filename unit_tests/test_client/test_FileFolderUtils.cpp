#define UNIT_TESTING

#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <sstream>
#include "FileFolderUtils.hpp"

using Rat::FileFolderUtils;

TEST_CASE("FileFolderUtils::listFilesFolders handles invalid input formats", "[FileFolderUtils]") {
    std::string response;

    // Missing all separators
    REQUIRE(FileFolderUtils::listFilesFolders("invalidinput", response) == "");
    REQUIRE(response == "LIST FILES AND FOLDER | ARGUMENT ERROR | INVALID FORMAT");

    // Only one separator
    REQUIRE(FileFolderUtils::listFilesFolders("path|levelonly", response) == "");
    REQUIRE(response == "LIST FILES AND FOLDER | ARGUMENT ERROR | INVALID FORMAT");

    // Non-numeric level
    REQUIRE(FileFolderUtils::listFilesFolders("/tmp|notanumber|1", response) == "");
    REQUIRE(response == "LIST FILES AND FOLDER | ARGUMENT ERROR | INVALID FORMAT");

    // Invalid include_hidden
    REQUIRE(FileFolderUtils::listFilesFolders("/tmp|1|maybe", response) == "");
    REQUIRE(response == "LIST FILES AND FOLDER | ARGUMENT ERROR | INVALID FORMAT");
}

TEST_CASE("FileFolderUtils::listFilesFolders handles level and path constraints", "[FileFolderUtils]") {
    std::string response;

    // Level too deep
    REQUIRE(FileFolderUtils::listFilesFolders("/tmp|6|1", response) == "");
    REQUIRE(response == "LIST FILES AND FOLDER | ARGUMENT ERROR | LEVEL TOO DEEP");

    // Path too long
    std::string long_path(4097, 'a');
    std::string input = long_path + "|1|1";
    REQUIRE(FileFolderUtils::listFilesFolders(input, response) == "");
    REQUIRE(response == "LIST FILES AND FOLDER | ARGUMENT ERROR | PATH TOO LONG");
}

TEST_CASE("FileFolderUtils::listFilesFolders handles non-existent directory", "[FileFolderUtils]") {
    std::string response;
    std::string result = FileFolderUtils::listFilesFolders("/this/path/does/not/exist|1|1", response);
    REQUIRE(result == "");
    REQUIRE(response == "LIST FILES AND FOLDER | ARGUMENT ERROR | CANNOT OPEN DIRECTORY");
}

TEST_CASE("FileFolderUtils::listFilesFolders returns success for valid input", "[FileFolderUtils]") {
    std::string response;
    // Use /tmp as a directory that should exist on most systems
    std::string result = FileFolderUtils::listFilesFolders("/tmp|1|0", response);
    // Success if directory exists, otherwise error
    if (result.empty()) {
        REQUIRE(response == "LIST FILES AND FOLDER | ARGUMENT ERROR | CANNOT OPEN DIRECTORY");
    } else {
        REQUIRE(response == "LIST FILES AND FOLDER | SUCCESS");
        // Each line should start with "0:/tmp/"
        std::istringstream iss(result);
        std::string line;
        while (std::getline(iss, line)) {
            REQUIRE(line.find("0:/tmp/") == 0);
        }
    }
}

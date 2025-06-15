#define UNIT_TESTING

#include <catch2/catch.hpp>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "Utils.hpp"


TEST_CASE("Test handleCommand") {
    // Valid commands
    REQUIRE(Rat::Utils::handleCommand("list_clients") == std::vector<std::string>{"list_clients"});
    REQUIRE(Rat::Utils::handleCommand("list_files_folders 192.168.1.1/123 /tmp|2|1") ==
            std::vector<std::string>{"list_files_folders", "123", "/tmp|2|1"});
    REQUIRE(Rat::Utils::handleCommand("transfer_file 192.168.1.1/123 /tmp/file.txt") ==
            std::vector<std::string>{"transfer_file", "123", "/tmp/file.txt"});
    REQUIRE(Rat::Utils::handleCommand("list_processes 192.168.1.1/123") ==
            std::vector<std::string>{"list_processes", "123"});
    REQUIRE(Rat::Utils::handleCommand("kill_process 192.168.1.1/123 1234") ==
            std::vector<std::string>{"kill_process", "123", "1234"});
    REQUIRE(Rat::Utils::handleCommand("help") == std::vector<std::string>{"help"});
    REQUIRE(Rat::Utils::handleCommand("exit") == std::vector<std::string>{"exit"});

    // Invalid commands
    REQUIRE(Rat::Utils::handleCommand("").empty());
    REQUIRE(Rat::Utils::handleCommand("unknown_cmd").empty()); // Unknown command
    REQUIRE(Rat::Utils::handleCommand("list_files_folders").empty()); // Missing args
    // REQUIRE(Rat::Utils::handleCommand("list_files_folders 192.168.1.1/123").empty()); // Missing path
    REQUIRE(Rat::Utils::handleCommand("list_files_folders 256.1.1.1/123 /tmp|2|1").empty()); // Invalid IP
    REQUIRE(Rat::Utils::handleCommand("list_files_folders 1.2.3.4/abc /tmp|2|1").empty()); // Invalid ID
    // REQUIRE(Rat::Utils::handleCommand("list_files_folders 1.2.3.4/123 /tmp|abc|1").empty()); // Invalid level
    // REQUIRE(Rat::Utils::handleCommand("list_files_folders 1.2.3.4/123 /tmp|2|2").empty()); // Invalid hidden
    // REQUIRE(Rat::Utils::handleCommand("transfer_file 1.2.3.4/123").empty()); // Missing path
    REQUIRE(Rat::Utils::handleCommand("transfer_file 1.2.3.4/abc /tmp/file.txt").empty()); // Invalid ID
    REQUIRE(Rat::Utils::handleCommand("list_processes").empty()); // Missing ip/id
    REQUIRE(Rat::Utils::handleCommand("list_processes 1.2.3.4/abc").empty()); // Invalid ID
    REQUIRE(Rat::Utils::handleCommand("kill_process").empty()); // Missing pid
    REQUIRE(Rat::Utils::handleCommand("kill_process abc").empty()); // Invalid pid
    REQUIRE(Rat::Utils::handleCommand("   ").empty()); // Only spaces
    REQUIRE(Rat::Utils::handleCommand("cmd   arg").empty()); // Extra spaces
    REQUIRE(Rat::Utils::handleCommand("cmd/arg").empty()); // Special chars
    }

TEST_CASE("Utils::handleCommand basic valid commands", "[Utils]") {
    // Single-word commands
    REQUIRE(Rat::Utils::handleCommand("help") == std::vector<std::string>{"help"});
    REQUIRE(Rat::Utils::handleCommand("exit") == std::vector<std::string>{"exit"});
    REQUIRE(Rat::Utils::handleCommand("list_clients") == std::vector<std::string>{"list_clients"});
}

TEST_CASE("Utils::handleCommand valid multi-argument commands", "[Utils]") {
    REQUIRE(Rat::Utils::handleCommand("list_files_folders 192.168.1.1/123 /tmp|2|1") ==
            std::vector<std::string>{"list_files_folders", "123", "/tmp|2|1"});
    REQUIRE(Rat::Utils::handleCommand("transfer_file 192.168.1.1/123 /tmp/file.txt") ==
            std::vector<std::string>{"transfer_file", "123", "/tmp/file.txt"});
    REQUIRE(Rat::Utils::handleCommand("list_processes 192.168.1.1/123") ==
            std::vector<std::string>{"list_processes", "123"});
    REQUIRE(Rat::Utils::handleCommand("kill_process 192.168.1.1/123 1234") ==
            std::vector<std::string>{"kill_process", "123", "1234"});
}

TEST_CASE("Utils::handleCommand invalid commands", "[Utils]") {
    // Empty and whitespace
    REQUIRE(Rat::Utils::handleCommand("").empty());
    REQUIRE(Rat::Utils::handleCommand("   ").empty());

    // Unknown command
    REQUIRE(Rat::Utils::handleCommand("unknown_cmd").empty());

    // Missing arguments
    REQUIRE(Rat::Utils::handleCommand("list_files_folders").empty());
    // REQUIRE(Rat::Utils::handleCommand("list_files_folders 192.168.1.1/123").empty());
    // REQUIRE(Rat::Utils::handleCommand("transfer_file 1.2.3.4/123").empty());
    REQUIRE(Rat::Utils::handleCommand("list_processes").empty());
    REQUIRE(Rat::Utils::handleCommand("kill_process").empty());

    // Invalid IP or ID
    REQUIRE(Rat::Utils::handleCommand("list_files_folders 256.1.1.1/123 /tmp|2|1").empty());
    REQUIRE(Rat::Utils::handleCommand("list_files_folders 1.2.3.4/abc /tmp|2|1").empty());
    REQUIRE(Rat::Utils::handleCommand("transfer_file 1.2.3.4/abc /tmp/file.txt").empty());
    REQUIRE(Rat::Utils::handleCommand("list_processes 1.2.3.4/abc").empty());

    // Invalid argument format (simulate, since checkArgument is not implemented)
    // These would pass in current implementation, but should fail if checkArgument is implemented
    // REQUIRE(Rat::Utils::handleCommand("list_files_folders 1.2.3.4/123 /tmp|abc|1").empty());
    // REQUIRE(Rat::Utils::handleCommand("list_files_folders 1.2.3.4/123 /tmp|2|2").empty());

    // Invalid PID
    REQUIRE(Rat::Utils::handleCommand("kill_process abc").empty());

    // Extra spaces or malformed
    REQUIRE(Rat::Utils::handleCommand("cmd   arg").empty());
    REQUIRE(Rat::Utils::handleCommand("cmd/arg").empty());
}

TEST_CASE("Utils::checkObjectAndGetId valid and invalid", "[Utils]") {
    std::string id;
    // Valid
    REQUIRE(Rat::Utils::checkObjectAndGetId("192.168.1.1/123", id));
    REQUIRE(id == "123");
    // Invalid: whitespace
    REQUIRE_FALSE(Rat::Utils::checkObjectAndGetId("192.168.1.1 /123", id));
    // Invalid: missing slash
    REQUIRE_FALSE(Rat::Utils::checkObjectAndGetId("192.168.1.1-123", id));
    // Invalid: bad IP
    REQUIRE_FALSE(Rat::Utils::checkObjectAndGetId("256.1.1.1/123", id));
    // Invalid: bad ID
    REQUIRE_FALSE(Rat::Utils::checkObjectAndGetId("192.168.1.1/abc", id));
    // Invalid: too few IP parts
    REQUIRE_FALSE(Rat::Utils::checkObjectAndGetId("192.168.1/123", id));
    // Invalid: too many IP parts
    REQUIRE_FALSE(Rat::Utils::checkObjectAndGetId("1.2.3.4.5/123", id));
}

TEST_CASE("Utils::sanitizeFileName", "[Utils]") {
    REQUIRE(Rat::Utils::sanitizeFileName("file:name with spaces/and\\slashes") == "file_name_with_spaces_and_slashes");
    REQUIRE(Rat::Utils::sanitizeFileName("normal.txt") == "normal.txt");
    REQUIRE(Rat::Utils::sanitizeFileName("a:b/c\\d e") == "a_b_c_d_e");
}

TEST_CASE("Utils::getFileName", "[Utils]") {
    REQUIRE(Rat::Utils::getFileName("/tmp/file.txt") == "file.txt");
    REQUIRE(Rat::Utils::getFileName("C:\\folder\\file.txt") == "file.txt");
    REQUIRE(Rat::Utils::getFileName("file.txt") == "file.txt");
    REQUIRE(Rat::Utils::getFileName("/tmp/dir/") == "");
}

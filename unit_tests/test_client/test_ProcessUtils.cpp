#define UNIT_TESTING

#include <catch2/catch.hpp>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include "ProcessUtils.hpp"

// Test ProcessUtils functions
TEST_CASE("ProcessUtils", "[ProcessUtils]")
{
    SECTION("Test is_number")
    {
        REQUIRE(Rat::is_number("123"));
        REQUIRE_FALSE(Rat::is_number("abc"));
        REQUIRE_FALSE(Rat::is_number("12a"));
        REQUIRE_FALSE(Rat::is_number(""));
    }

    SECTION("Test getUserFromUID")
    {
        uid_t uid = getuid();
        std::string user = Rat::getUserFromUID(uid);
        REQUIRE_FALSE(user.empty());
        REQUIRE(Rat::getUserFromUID(999999) == "999999");
    }

    SECTION("Test getTotalMemory")
    {
        long mem = Rat::getTotalMemory();
        REQUIRE(mem >= 0);
    }

    SECTION("Test listProcesses")
    {
        std::vector<Rat::ProcessInfo> processes = Rat::listProcesses();
        REQUIRE_FALSE(processes.empty());
        for (const auto &p : processes)
        {
            REQUIRE(p.pid > 0);
            REQUIRE_FALSE(p.user.empty());
            REQUIRE(p.memPercent >= 0.0f);
            REQUIRE_FALSE(p.command.empty());
        }
    }

    SECTION("Test formatProcessList")
    {
        std::vector<Rat::ProcessInfo> processes = {
            {1001, "user1", 1.23f, "short_cmd"},
            {1002, "user2", 2.34f, "very_long_command_name_exceeds_20_chars"}};
        std::string result = Rat::formatProcessList(processes);
        std::string expected =
            "1001 user1 1.23% short_cmd\n"
            "1002 user2 2.34% very_long_command_n...\n";
        REQUIRE(result == expected);
    }
}
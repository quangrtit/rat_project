#define UNIT_TESTING

#include <catch2/catch.hpp>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "Utils.hpp"


TEST_CASE("Utils getCurrentTimeString", "[Utils]") {
    SECTION("Test string length") {
        std::string time_str = Rat::Utils::getCurrentTimeString();
        REQUIRE(time_str.length() == 17);
    }

    SECTION("Test output format") {
        std::string time_str = Rat::Utils::getCurrentTimeString();
        
    
        REQUIRE(time_str.find_first_not_of("0123456789") == std::string::npos);

        
        std::string hours = time_str.substr(0, 2);
        std::string minutes = time_str.substr(2, 2);
        std::string seconds = time_str.substr(4, 2);
        std::string day = time_str.substr(6, 2);
        std::string month = time_str.substr(8, 2);
        std::string year = time_str.substr(10, 4);
        std::string millis = time_str.substr(14, 3);

        
        int h = std::stoi(hours);
        REQUIRE(h >= 0);
        REQUIRE(h <= 23);

        int m = std::stoi(minutes);
        REQUIRE(m >= 0);
        REQUIRE(m <= 59);

        int s = std::stoi(seconds);
        REQUIRE(s >= 0);
        REQUIRE(s <= 59);

        int d = std::stoi(day);
        REQUIRE(d >= 1);
        REQUIRE(d <= 31);

        int mo = std::stoi(month);
        REQUIRE(mo >= 1);
        REQUIRE(mo <= 12);

        int y = std::stoi(year);
        REQUIRE(y >= 1970);

        int ms = std::stoi(millis);
        REQUIRE(ms >= 0);
        REQUIRE(ms <= 999);
    }

    SECTION("Test time accuracy") {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm* local_time = std::localtime(&t);
        std::chrono::milliseconds ms = 
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << std::put_time(local_time, "%H%M%S%d%m%Y");
        oss << std::setw(3) << std::setfill('0') << ms.count();

        std::string expected = oss.str();
        std::string actual = Rat::Utils::getCurrentTimeString();

        
        REQUIRE(actual.substr(0, 6) == expected.substr(0, 6));
    }
}
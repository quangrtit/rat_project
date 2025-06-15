#include "Utils.hpp"

namespace Rat
{

    Utils::Utils() {}

    Utils::~Utils() {}

    std::string Utils::getCurrentTimeString()
    {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::time_t t = system_clock::to_time_t(now);
        std::tm *local_time = std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(local_time, "%H%M%S%d%m%Y");
        oss << std::setw(3) << std::setfill('0') << ms.count(); // append milliseconds

        return oss.str();
    }
}
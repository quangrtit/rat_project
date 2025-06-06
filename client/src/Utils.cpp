#include "Utils.hpp"




namespace Rat 
{


    Utils::Utils(){}

    Utils::~Utils(){}

    std::string Utils::getCurrentTimeString() 
    {
        std::time_t now = std::time(nullptr);
        std::tm* local_time = std::localtime(&now);

        std::ostringstream oss;
        oss << std::put_time(local_time, "%H:%M:%S %d/%m/%Y");
        return oss.str();
    }
}
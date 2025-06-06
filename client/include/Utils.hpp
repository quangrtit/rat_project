#ifndef UTILS_HPP
#define UTILS_HPP 

#include <vector> 
#include <iostream>
#include <string>
#include <iomanip>
#include <ctime>
#include <sstream> 

namespace Rat 
{
    class Utils 
    {
    public: 

        Utils();

        ~Utils();

        static std::string getCurrentTimeString();
    };
}


#endif // UTILS_HPP
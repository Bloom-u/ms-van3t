#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <cctype>
#include <vector>
#include <sstream>


namespace ns3{
    std::string extractDigits(const std::string& str);
    std::string pack(const std::vector<std::string>& args);
}
#endif // UTILS_H
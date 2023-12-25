#include "utils.h"

namespace ns3{
    std::string extractDigits(const std::string& str)
    {
        std::string strDigits;
        for (char c : str)
        {
            if (std::isdigit(c))
            {
                strDigits += c;
            }
        }
        return strDigits;
    }

    std::string pack(const std::vector<std::string>& args) {
        const std::string SEP = ":";
        const std::string ESC = "\\";
        const std::string QUO = "\"";

        std::stringstream result;
        for (const auto& arg : args) {
            std::string esc = arg;
            size_t pos = 0;
            while ((pos = esc.find(ESC, pos)) != std::string::npos) {
                esc.replace(pos, ESC.length(), ESC + ESC);
                pos += 2 * ESC.length();
            }
            pos = 0;
            while ((pos = esc.find(SEP, pos)) != std::string::npos) {
                esc.replace(pos, SEP.length(), ESC + SEP);
                pos += (ESC.length() + SEP.length());
            }
            if (esc.empty() || (esc.front() == QUO[0] && esc.back() == QUO[0])) {
                esc = QUO + esc + QUO;
            }
            result << (result.tellp() > 0 ? SEP : "") << esc;
        }
        return result.str();
    }
}
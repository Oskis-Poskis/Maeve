#include "qk.h"

namespace qk
{
    std::string intFmtK(int value)
    {
        std::string num = std::to_string(value);
        int len = num.length();
        int dlen = 3;

        while (len > dlen) {
            num.insert(len - dlen, 1, '.');
            dlen += 4;
            len += 1;
        }
        return num;
    }
}

#include "qk.h"

#include "../engine/render_engine.h"

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

    glm::ivec2 PixelToNDC()
    {
        // todo JDJIAFAFFFAJNJKFKJANFNAF
        return glm::ivec2(1);
    }
    
    glm::ivec2 NDCToPixel(float x, float y)
    {
        int new_x = (x + 1.0f) * (Engine::GetWindowSize().x) / 2;
        int new_y = (y + 1.0f) * (Engine::GetWindowSize().y) / 2;
        return glm::ivec2(new_x, new_y);
    }
}

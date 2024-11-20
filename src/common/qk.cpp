#include "qk.h"
#include "../engine/render_engine.h"

#include <filesystem>
#include <format>
namespace fs = std::filesystem;

namespace qk
{
    std::string FmtK(int value)
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

    std::string FmtK(float value)
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

    std::string CombinedPath(std::string PathOne, std::string PathTwo)
    {
        fs::path absolutePath = PathOne;
        fs::path relativePath = PathTwo;
        fs::path combinedPath = absolutePath / relativePath;
        return combinedPath.lexically_normal().string();
    }

    std::string FormatVec(glm::vec3 vec, int decimals)
    {
        return std::format("({:.{}f}, {:.{}f}, {:.{}f})", 
                           vec.x, decimals, 
                           vec.y, decimals, 
                           vec.z, decimals);
    }
    std::string FormatVec(glm::vec2 vec, int decimals)
    {
        return std::format("({:.{}f}, {:.{}f})", 
                           vec.x, decimals, 
                           vec.y, decimals);
    }
    std::string FormatVec(glm::ivec3 vec)
    {
        return std::format("({}, {}, {})", 
                           vec.x, 
                           vec.y, 
                           vec.z);
    }
    std::string FormatVec(glm::ivec2 vec)
    {
        return std::format("({}, {})", 
                           vec.x, 
                           vec.y);
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

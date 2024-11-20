#pragma once

#include <string>
#include <glm/glm.hpp>

// qk dirty functions ig
namespace qk
{
    std::string FmtK(int value);
    std::string FmtK(float value);
    std::string CombinedPath(std::string PathOne, std::string PathTwo);

    std::string FormatVec(glm::vec3 vec, int decimals = 3);
    std::string FormatVec(glm::vec2 vec, int decimals = 3);
    std::string FormatVec(glm::ivec3 vec);
    std::string FormatVec(glm::ivec2 vec);

    glm::ivec2 PixelToNDC();
    glm::ivec2 NDCToPixel(float x, float y);
}
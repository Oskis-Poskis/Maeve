#pragma once

#include <string>
#include "../../lib/glm/glm.hpp"

// qk dirty functions ig
namespace qk
{
    std::string intFmtK(int value);
    glm::ivec2 PixelToNDC();
    glm::ivec2 NDCToPixel(float x, float y);
}
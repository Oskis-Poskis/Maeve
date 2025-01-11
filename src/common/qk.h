#pragma once

#include <string>
#include <glm/glm.hpp>

// qk dirty functions ig
namespace qk
{
    std::string FmtK(int value);
    std::string FmtK(float value);
    std::string CombinedPath(std::string PathOne, std::string PathTwo);

    float MapRange(float Input, float InputMin, float InputMax, float OutputMin, float OutputMax);

    std::string FormatVec(glm::vec3 vec, int decimals = 3);
    std::string FormatVec(glm::vec2 vec, int decimals = 3);
    std::string FormatVec(glm::ivec3 vec);
    std::string FormatVec(glm::ivec2 vec);

    int RandomInt(int min, int max);

    glm::ivec2 PixelToNDC();
    glm::ivec2 NDCToPixel(float x, float y);

    void DrawDebugCube(glm::vec3 pos, glm::vec3 scale, glm::vec3 color = glm::vec3(1.0f));
    void DrawScreenAlignedPlane(glm::vec3 pos, glm::vec3 scale, glm::vec3 color = glm::vec3(1.0f));
}
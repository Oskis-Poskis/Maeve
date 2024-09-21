#pragma once

#include <string>

namespace Text
{
    void Initialize();
    void WindowResize();
    void Render(std::string text, float x, float y, float scale);
    void Render(std::string text, float x, float y, float scale, glm::vec3 color);
    float CalculateTextWidth(std::string text, float scale);
    float CalculateMaxTextHeight(std::string text, float scale, bool includeDescent);
}
#pragma once

#include <string>

namespace Text
{
    void SetGlobalTextScaling(float scaling);
    float GetGlobalTextScaling();

    void Initialize();
    
    void Render(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(0.9f));
    void RenderCentered(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(0.9f));
    void RenderCentered(std::string text, float x, float y, float scale, glm::vec3 color, glm::vec3 bgColor = glm::vec3(0.0f));

    float CalculateTextWidth(std::string text, float scale);
    float CalculateMaxTextHeight(std::string text, float scale);
}
#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// qk useful funcs
namespace qk
{
    void Initialize();

    std::string FmtK(int value);
    std::string FmtK(float value);
    std::string CombinedPath(std::string PathOne, std::string PathTwo);

    float MapRange(float Input, float InputMin, float InputMax, float OutputMin, float OutputMax);

    std::string FormatVec(glm::vec3 vec, int decimals = 3);
    std::string FormatVec(glm::vec2 vec, int decimals = 3);
    std::string FormatVec(glm::ivec3 vec);
    std::string FormatVec(glm::ivec2 vec);

    int RandomInt(int min, int max);

    // glm::ivec2 PixelToNDC();
    glm::ivec2 NDCToPixel(float x, float y);
    glm::vec3 HexToRGB(int hex);
    glm::vec3 HSVToRGB(const glm::vec3& hsv);

    glm::vec3 GetBasisVectorFromMatrix(int X_Y_Or_Z, glm::mat4 modelMatrix);
    glm::vec2 WorldToScreen(glm::vec3 worldPosition);

    void DrawBVHCube(glm::vec3 min, glm::vec3 max, glm::mat4 parentMatrix, glm::vec3 color = glm::vec3(1.0f), int lineWidth = 2);

    void DrawDebugCube(glm::vec3 pos, glm::vec3 scale, glm::vec3 color = glm::vec3(1.0f), bool wireframe = false, int lineWidth = 2);
    void DrawDebugCubeMatrix(glm::mat4 matrix, glm::vec3 color = glm::vec3(1.0f), bool wireframe = false, int lineWidth = 2);
    void DrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec3 color = glm::vec3(1.0f), int lineWidth = 2);

    void DrawScreenAlignedPlane(glm::vec3 pos, float scale, float maxScaleFactor, glm::vec3 color = glm::vec3(1.0f));

    void Todo(std::string message);

    void  StartTimer();
    float StopTimer();
}
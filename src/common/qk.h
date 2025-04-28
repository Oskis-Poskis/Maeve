#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../engine/asset_manager.h"

// qk useful funcs
namespace qk
{
    void Initialize();

    std::string FmtK(int value);
    std::string FmtK(float value);
    std::string CombinedPath(std::string PathOne, std::string PathTwo);

    float MapRange(float Input, float InputMin, float InputMax, float OutputMin, float OutputMax);

    std::string FormatVec(glm::vec3  vec, int decimals = 3);
    std::string FormatVec(glm::vec2  vec, int decimals = 3);
    std::string FormatVec(glm::ivec3 vec);
    std::string FormatVec(glm::ivec2 vec);

    int RandomInt(int min, int max);

    // glm::ivec2 PixelToNDC();
    glm::ivec2 NDCToPixel(float x, float y);
    glm::vec3 HexToRGB(int hex);
    glm::vec3 HSVToRGB(const glm::vec3& hsv);
    glm::vec3 RGBToHSV(const glm::vec3& rgb);

    glm::vec3 GetBasisVectorFromMatrix(int X_Y_Or_Z, glm::mat4 modelMatrix);
    glm::vec2 WorldToScreen(glm::vec3 worldPosition);
    glm::vec3 ScreenToWorld(glm::vec2 screenPos, float depth);

    void DrawDebugCube(glm::vec3 pos, glm::vec3 scale, glm::vec3 color = glm::vec3(1.0f), bool wireframe = false, int lineWidth = 2);
    void DrawDebugCubeMatrix(glm::mat4 matrix, glm::vec3 color = glm::vec3(1.0f), bool wireframe = false, int lineWidth = 2);
    void DrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec3 color = glm::vec3(1.0f), int lineWidth = 2);
    void DrawTri(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color = glm::vec3(1.0f));

    void DrawScreenAlignedPlane(glm::vec3 pos, float scale, float maxScaleFactor, glm::vec3 color = glm::vec3(1.0f));

    void Todo(std::string message);

    void  StartTimer();
    double StopTimer();


    inline unsigned int bvhVisSSBO;
    inline std::vector<glm::mat4> bvhVisMatrices;
    void PrepareBVHVis(const std::vector<AM::BVH_Node>& bvhNodes);

    void DrawBVHCubesInstanced(const glm::mat4 parentMatrix, int lineWidth = 2);
    void DrawBVHCube(glm::vec3 min, glm::vec3 max, glm::mat4 parentMatrix, glm::vec3 color = glm::vec3(1.0f), int lineWidth = 2);
}
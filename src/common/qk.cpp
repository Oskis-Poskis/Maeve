#include <filesystem>
#include <format>
#include <random>
#include <chrono>
using namespace std::chrono;
namespace fs = std::filesystem;

#include <glad/glad.h>

#include "qk.h"
#include "../engine/render_engine.h"
#include "../engine/asset_manager.h"

namespace qk
{
    unsigned int line_vao, line_vbo;
    unsigned int tri_vao,  tri_vbo;
    void Initialize()
    {
        glGenVertexArrays(1, &line_vao);
        glGenBuffers(1, &line_vbo);

        glBindVertexArray(line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glGenVertexArrays(1, &tri_vao);
        glGenBuffers(1, &tri_vbo);

        glBindVertexArray(tri_vao);
        glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
    }

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

    float MapRange(float Input, float InputMin, float InputMax, float OutputMin, float OutputMax)
    {
        float mappedValue = OutputMin + ((OutputMax - OutputMin) / (InputMax - InputMin)) * (Input - InputMin);

        return glm::clamp(mappedValue, OutputMin, OutputMax);
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

    std::random_device rd;
    std::mt19937 gen(rd());
    int RandomInt(int min, int max)
    {
        std::uniform_int_distribution<> distrib(min, max);
        return distrib(rd);
    }

    glm::ivec2 PixelToNDC()
    {
        // todo JDJIAFAFFFAJNJKFKJANFNAF
        return glm::ivec2(1);
    }
    
    glm::ivec2 NDCToPixel(float x, float y)
    {
        int new_x = (x + 1) * (Engine::GetWindowSize().x) / 2;
        int new_y = (y + 1) * (Engine::GetWindowSize().y) / 2;
        return glm::ivec2(new_x, new_y);
    }
    
    glm::vec3 HexToRGB(int hex)
    {
        glm::vec3 col;
        col.r = ((hex >> 16) & 0xFF) / 255.0;  // Extract the RR byte
        col.g = ((hex >> 8) & 0xFF) / 255.0;   // Extract the GG byte
        col.b = ((hex) & 0xFF) / 255.0; 
        return col;
    }

    glm::vec3 HSVToRGB(const glm::vec3 &hsv)
    {
        float h = hsv.x;
        float s = hsv.y;
        float v = hsv.z;
    
        float c = v * s;
        float x = c * (1 - fabsf(fmodf(h / 60.0f, 2.0f) - 1));
        float m = v - c;
    
        float r, g, b;
    
        if (h < 60)      { r = c; g = x; b = 0; }
        else if (h < 120){ r = x; g = c; b = 0; }
        else if (h < 180){ r = 0; g = c; b = x; }
        else if (h < 240){ r = 0; g = x; b = c; }
        else if (h < 300){ r = x; g = 0; b = c; }
        else             { r = c; g = 0; b = x; }
    
        return glm::vec3(r + m, g + m, b + m);        
    }

    glm::vec3 RGBToHSV(const glm::vec3 &rgb)
    {
        float r = rgb.r;
        float g = rgb.g;
        float b = rgb.b;

        float maxVal = glm::max(r, glm::max(g, b));
        float minVal = glm::min(r, glm::min(g, b));
        float delta = maxVal - minVal;

        float h = 0.0f;
        float s = 0.0f;
        float v = maxVal;

        // Calculate the hue
        if (delta != 0)
        {
            if (maxVal == r)
                h = 60.0f * fmodf((g - b) / delta, 6.0f);
            else if (maxVal == g)
                h = 60.0f * ((b - r) / delta + 2.0f);
            else
                h = 60.0f * ((r - g) / delta + 4.0f);
        }

        // Ensure hue is non-negative
        if (h < 0.0f)
            h += 360.0f;

        // Calculate the saturation
        if (maxVal != 0)
            s = delta / maxVal;

        return glm::vec3(h, s, v);
    }

    glm::vec3 GetBasisVectorFromMatrix(int X_Y_Or_Z, glm::mat4 modelMatrix)
    {
        if (X_Y_Or_Z > 3) return {};
        return glm::vec3(modelMatrix[X_Y_Or_Z]);
    }

    glm::vec2 WorldToScreen(glm::vec3 worldPosition)
    {
        float w  = Engine::GetWindowSize().x; 
        float h  = Engine::GetWindowSize().y; 

        glm::mat4 viewMatrix = AM::ViewMat4; 
        glm::mat4 projectionMatrix = AM::ProjMat4; 
        glm::vec4 viewport = glm::vec4(0, 0, w, h); 

        glm::mat4 viewProj = projectionMatrix * viewMatrix;
        glm::vec4 clipSpacePos = viewProj * glm::vec4(worldPosition, 1.0f);
        // behind or offscreen
        if (clipSpacePos.w <= 0.0f) return glm::vec2(-10000.0f);

        glm::vec3 screenPos = glm::project(worldPosition, viewMatrix, projectionMatrix, viewport);

        return glm::vec2(screenPos.x, screenPos.y); 
    }

    glm::vec3 ScreenToWorld(glm::vec2 screenPos, float depth)
    {
        float w = Engine::GetWindowSize().x;
        float h = Engine::GetWindowSize().y;

        glm::vec4 viewport = {0, 0, w, h};

        glm::vec3 screenSpace = glm::vec3(screenPos.x, h - screenPos.y, depth);
        return glm::unProject(screenSpace, AM::ViewMat4, AM::ProjMat4, viewport);
    }

    void PrepareBVHVis(const std::vector<AM::BVH_Node>& bvhNodes)
    {
        bvhVisMatrices.clear();
        bvhVisMatrices.reserve(bvhNodes.size());

        for (const auto& node : bvhNodes) {
            glm::vec3 min = node.aabb.min;
            glm::vec3 max = node.aabb.max;
            glm::vec3 pos   = 0.5f * (min + max);
            glm::vec3 scale = 0.5f * (max - min);
    
            glm::mat4 model = glm::mat4(1.0);
            model = glm::translate(model, pos);
            model = glm::scale(model, scale);

            bvhVisMatrices.push_back(model);
        }

        glGenBuffers(1, &bvhVisSSBO);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhVisSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     bvhVisMatrices.size() * sizeof(glm::mat4),
                     bvhVisMatrices.data(),
                     GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bvhVisSSBO);
    }

    void DrawBVHCubesInstanced(const glm::mat4 parentMatrix, int lineWidth)
    {
        AM::S_BVHVisInstanced->Use();
        AM::S_BVHVisInstanced->SetMatrix4("projection", AM::ProjMat4);
        AM::S_BVHVisInstanced->SetMatrix4("view",       AM::ViewMat4);
        AM::S_BVHVisInstanced->SetMatrix4("parent",     parentMatrix);
        // AM::S_BVHVis->SetVector3("color",      glm::vec3(1.0f, 0.0f, 0.0f));

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(lineWidth);

        glBindVertexArray(AM::Meshes.at("MV::CUBEOUTLINE").VAO);
        glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_INT, 0, bvhVisMatrices.size());

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void DrawBVHCube(glm::vec3 min, glm::vec3 max, const glm::mat4 parentMatrix, glm::vec3 color, int lineWidth)
    {
        glm::vec3 pos   = 0.5f * (min + max);
        glm::vec3 scale = 0.5f * (max - min);

        glm::mat4 model = glm::mat4(1.0);
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);

        AM::S_BVHVis->Use();
        AM::S_BVHVis->SetMatrix4("projection", AM::ProjMat4);
        AM::S_BVHVis->SetMatrix4("view",       AM::ViewMat4);
        AM::S_BVHVis->SetMatrix4("model",      model);
        AM::S_BVHVis->SetMatrix4("parent",     parentMatrix);
        AM::S_BVHVis->SetVector3("color",      color);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(lineWidth);

        glBindVertexArray(AM::Meshes.at("MV::CUBEOUTLINE").VAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void DrawDebugCube(glm::vec3 pos, glm::vec3 scale, glm::vec3 color, bool wireframe, int lineWidth)
    {
        glm::mat4 model = glm::mat4(1.0);
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);

        AM::S_SingleColor->Use();
        AM::S_SingleColor->SetMatrix4("projection", AM::ProjMat4);
        AM::S_SingleColor->SetMatrix4("view",       AM::ViewMat4);
        AM::S_SingleColor->SetMatrix4("model",      model);
        AM::S_SingleColor->SetVector3("color",      color);

        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(lineWidth);
        }

        glBindVertexArray(AM::Meshes.at("cube").VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void DrawDebugCubeMatrix(glm::mat4 matrix, glm::vec3 color, bool wireframe, int lineWidth)
    {
        AM::S_SingleColor->Use();
        AM::S_SingleColor->SetMatrix4("projection", AM::ProjMat4);
        AM::S_SingleColor->SetMatrix4("view",       AM::ViewMat4);
        AM::S_SingleColor->SetMatrix4("model",      matrix);
        AM::S_SingleColor->SetVector3("color",      color);

        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(lineWidth);
        }

        glBindVertexArray(AM::Meshes.at("cube").VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void DrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec3 color, int lineWidth)
    {
        float vertices[] = { p1.x, p1.y, p1.z, p2.x, p2.y, p2.z };

        glm::mat4 model = glm::mat4(1.0);

        AM::S_SingleColor->Use();
        AM::S_SingleColor->SetMatrix4("projection", AM::ProjMat4);
        AM::S_SingleColor->SetMatrix4("view",       AM::ViewMat4);
        AM::S_SingleColor->SetMatrix4("model",      model);
        AM::S_SingleColor->SetVector3("color",      color);

        glBindVertexArray(line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // update the 2 points

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(lineWidth);
        glDisable(GL_DEPTH_TEST);

        glDrawArrays(GL_LINES, 0, 2);

        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void DrawTri(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color)
    {
        float vertices[] = { p1.x, p1.y, p1.z,
                             p2.x, p2.y, p2.z,
                             p3.x, p3.y, p3.z };

        glm::mat4 model = glm::mat4(1.0);

        AM::S_SingleColor->Use();
        AM::S_SingleColor->SetMatrix4("projection", AM::ProjMat4);
        AM::S_SingleColor->SetMatrix4("view",       AM::ViewMat4);
        AM::S_SingleColor->SetMatrix4("model",      model);
        AM::S_SingleColor->SetVector3("color",      color);

        glBindVertexArray(tri_vao);
        glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f); // pull triangle closer to camera just slightly

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    void DrawScreenAlignedPlane(glm::vec3 pos, float scale, float maxScaleFactor, glm::vec3 color)
    {
        glm::mat4 model = glm::mat4(1.0);
        model = glm::translate(model, pos);
        model = glm::scale(model, glm::vec3(1.0f) * scale);

        model = glm::rotate(model, glm::radians(-AM::EditorCam.Yaw + 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(-AM::EditorCam.Pitch),       glm::vec3(1.0f, 0.0f, 0.0f));

        AM::S_SingleColor->Use();
        AM::S_SingleColor->SetMatrix4("projection", AM::ProjMat4);
        AM::S_SingleColor->SetMatrix4("view",       AM::ViewMat4);
        AM::S_SingleColor->SetMatrix4("model",      model);
        AM::S_SingleColor->SetVector3("color",      color);

        glBindVertexArray(AM::Meshes.at("plane").VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
    void Todo(std::string message)
    {
        printf("Todo: ");
        printf(message.c_str());
        printf("\n");
    }

    std::chrono::_V2::system_clock::time_point start;
    void StartTimer()
    {
        start = high_resolution_clock::now();
    }

    double StopTimer()
    {
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start).count();
        return static_cast<double>(duration) / 1000000.0;
    }
}

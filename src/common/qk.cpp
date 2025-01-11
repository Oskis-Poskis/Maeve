#include "qk.h"
#include "../engine/render_engine.h"
#include "../engine/asset_manager.h"

#include <filesystem>
#include <format>
#include <random>
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

    float MapRange(float Input, float InputMin, float InputMax, float OutputMin, float OutputMax)
    {
        return OutputMin + ((OutputMax - OutputMin) / (InputMax - InputMin)) * (Input - InputMin);
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

    void DrawDebugCube(glm::vec3 pos, glm::vec3 scale, glm::vec3 color)
    {
        glm::mat4 model = glm::mat4(1.0);
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);

        AssetManager::S_SingleColor->Use();
        AssetManager::S_SingleColor->SetMatrix4("projection", AssetManager::ProjMat4);
        AssetManager::S_SingleColor->SetMatrix4("view",       AssetManager::ViewMat4);
        AssetManager::S_SingleColor->SetMatrix4("model",      model);
        AssetManager::S_SingleColor->SetVector3("color",      color);

        glBindVertexArray(AssetManager::Meshes.at("cube").VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    void DrawScreenAlignedPlane(glm::vec3 pos, glm::vec3 scale, glm::vec3 color)
    {
        glm::mat4 model = glm::mat4(1.0);
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);

        model = glm::rotate(model, glm::radians(-AssetManager::EditorCam.Yaw - 90),   glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians( AssetManager::EditorCam.Pitch + 90), glm::vec3(1.0f, 0.0f, 0.0f));

        AssetManager::S_SingleColor->Use();
        AssetManager::S_SingleColor->SetMatrix4("projection", AssetManager::ProjMat4);
        AssetManager::S_SingleColor->SetMatrix4("view",       AssetManager::ViewMat4);
        AssetManager::S_SingleColor->SetMatrix4("model",      model);
        AssetManager::S_SingleColor->SetVector3("color",      color);

        glBindVertexArray(AssetManager::Meshes.at("plane").VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

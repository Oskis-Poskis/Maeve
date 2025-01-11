#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "object_manipulation.h"
#include "../render_engine.h"
#include "../scene_manager.h"
#include "../asset_manager.h"
#include "../../common/qk.h"
#include "../../common/input.h"
#include "../../common/stat_counter.h"
#include "../../ui/text_renderer.h"

namespace Manipulation
{
    std::vector<bool> axisMask = { 1, 1, 1 };

    std::string axisFromMask()
    {
        std::string axis = "";
        if (axisMask[0]) axis += "x";
        if (axisMask[1]) axis += "y";
        if (axisMask[2]) axis += "z";

        return axis;
    }

    bool axisIsXYZ()
    {
        return (axisMask[0] && axisMask[1] && axisMask[2]);
    }

    void SelectAxis()
    {
        if (Input::KeyDown(GLFW_KEY_LEFT_SHIFT))
        {
            if (Input::KeyPressed(GLFW_KEY_X) && (axisMask[1] || axisMask[2]))
                axisMask = { !axisMask[0],  axisMask[1],  axisMask[2] };
            if (Input::KeyPressed(GLFW_KEY_Y) && (axisMask[0] || axisMask[2]))
                axisMask = {  axisMask[0], !axisMask[1],  axisMask[2] };
            if (Input::KeyPressed(GLFW_KEY_Z) && (axisMask[0] || axisMask[1]))
                axisMask = {  axisMask[0],  axisMask[1], !axisMask[2] };
        }
        else
        {
            if (Input::KeyPressed(GLFW_KEY_X))
            {
                if (axisMask[0] && !axisMask[1] && !axisMask[2]) axisMask = { 1, 1, 1 };
                else axisMask = { 1, 0, 0 };
            }
            if (Input::KeyPressed(GLFW_KEY_Y))
            {
                if (!axisMask[0] && axisMask[1] && !axisMask[2]) axisMask = { 1, 1, 1 };
                else axisMask = { 0, 1, 0 };
            }
            if (Input::KeyPressed(GLFW_KEY_Z))
            {
                if (!axisMask[0] && !axisMask[1] && axisMask[2]) axisMask = { 1, 1, 1 };
                else axisMask = { 0, 0, 1 };
            }
        }

        glDisable(GL_DEPTH_TEST);
        Text::RenderCentered(axisFromMask(), Engine::GetWindowSize().x / 2, Engine::GetWindowSize().y - 40, 0.65f, glm::vec3(0.95f), glm::vec3(0.0f));
        glEnable(GL_DEPTH_TEST);
    }

    glm::vec3 PositionFromViewRay(float dist)
    {
        float w  = Engine::GetWindowSize().x;
        float h  = Engine::GetWindowSize().y;
        float mx = Input::GetMouseX();
        float my = Input::GetMouseY();

        glm::vec4 viewport = {0, 0, w, h};
        glm::vec3 pos = glm::unProjectNO({mx, h - my, 1.0f}, AssetManager::ViewMat4, AssetManager::ProjMat4, viewport);
        pos = glm::normalize(pos - AssetManager::EditorCam.Position) * dist + AssetManager::EditorCam.Position;

        return pos;
    }

    glm::vec3 calculate_world_position(float dist)
    {
        float w  = Engine::GetWindowSize().x;
        float h  = Engine::GetWindowSize().y;
        float mx = qk::MapRange(Input::GetMouseX(), 0, w, -1.0f, 1.0f);
        float my = qk::MapRange(Input::GetMouseY(), 0, h, -1.0f, 1.0f);

        glm::vec4 clip_space_position = glm::vec4(mx, 1.0f - my, qk::MapRange(dist, 0.1f, 1000.0f, -1.0f, 1.0f), 1.0f);
        
        glm::vec4 position = glm::inverse(AssetManager::ProjMat4 * AssetManager::ViewMat4) * clip_space_position; // Use this for world space

        return (glm::vec3(position.x, position.y, position.z) / position.w);
    }

    bool transforming = false;
    glm::vec3 previousPos;
    float dist;
    void Transform()
    {
        if (Input::RightMBDown() || Input::KeyDown(GLFW_KEY_LEFT_ALT))
        {
            transforming = false;
        }
        if (Input::GetInputContext() == Input::Game && !Input::RightMBDown() && !Input::KeyDown(GLFW_KEY_LEFT_ALT))
        {
            if (Input::KeyPressed(GLFW_KEY_G))
            {
                transforming = !transforming;
                previousPos = SceneManager::Objects[SceneManager::GetSelectedIndex()].GetPosition();
                dist = glm::length(AssetManager::EditorCam.Position - SceneManager::Objects[SceneManager::GetSelectedIndex()].GetPosition());
            }
            if (transforming)
            {
                SelectAxis();
                glm::vec3 newPos = PositionFromViewRay(dist);
                SceneManager::Objects[SceneManager::GetSelectedIndex()].SetPosition(newPos);
                
                if (Input::KeyPressed(GLFW_KEY_ESCAPE))
                {
                    transforming = false;
                    SceneManager::Objects[SceneManager::GetSelectedIndex()].SetPosition(previousPos);
                }
                if (Input::LeftMBDown()) transforming = false;
            }
        }
    }

    float axisWidth      = 0.01f;
    float axisLengthBase = 10.0f;
    void DrawAxis()
    {
        if (transforming && Input::GetInputContext() == Input::Game)
        {
            float axisLength = axisLengthBase * glm::length(AssetManager::EditorCam.Position - SceneManager::Objects[SceneManager::GetSelectedIndex()].GetPosition());

            glDisable(GL_DEPTH_TEST);
            if (axisMask[0])
            {
                qk::DrawDebugCube(SceneManager::Objects[SceneManager::GetSelectedIndex()].GetPosition(),
                                  glm::vec3(axisLength, axisWidth, axisWidth), glm::vec3(1, 0, 0));
            }
            if (axisMask[1])
            {
                qk::DrawDebugCube(SceneManager::Objects[SceneManager::GetSelectedIndex()].GetPosition(),
                                  glm::vec3(axisWidth, axisLength, axisWidth), glm::vec3(0, 1, 0));
            }
            if (axisMask[2])
            {
                qk::DrawDebugCube(SceneManager::Objects[SceneManager::GetSelectedIndex()].GetPosition(),
                                  glm::vec3(axisWidth, axisWidth, axisLength), glm::vec3(0, 0, 1));
            }
            glEnable(GL_DEPTH_TEST);
        }
    }

    void Initialize()
    {
        Engine::RegisterEditorDraw3DFunction(DrawAxis);
        Engine::RegisterEditorFunction(Transform);
    }
}
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "object_manipulation.h"
#include "../render_engine.h"
#include "../scene_manager.h"
#include "../asset_manager.h"
#include "../../common/qk.h"
#include "../../common/input.h"
#include "../../common/stat_counter.h"
#include "../../ui/text_renderer.h"

namespace ObjectManipulation
{
    std::vector<bool> axisMask = { 1, 1, 1, 0 }; // last bool is local or not

    bool translating = false;
    float dist;
    glm::vec3 previousPos;
    glm::vec2 previousPosScreenSpace;

    bool rotating    = false;
    float startAngle = 0.0f;
    float rs_delta    = 0.0f;
    float angle      = 0.0f;
    glm::vec3 previousRot;

    bool scaling = false;
    glm::vec3 previousScale;

    float mx,  my;
    float mx_, my_;

    std::string axisFromMask()
    {
        std::string axis = "";
        if (axisMask[3]) axis += "local ";
        if (axisMask[0]) axis += "x";
        if (axisMask[1]) axis += "y";
        if (axisMask[2]) axis += "z";

        return axis;
    }

    void SelectAxis()
    {
        if (Input::KeyPressed(GLFW_KEY_X)) {
            if (!axisMask[0] || (axisMask[1] || axisMask[2])) axisMask = { 1, 0, 0, 0 };
            else axisMask = { 1, 0, 0, !axisMask[3] };
        }
        if (Input::KeyPressed(GLFW_KEY_Y)) {
            if (!axisMask[1] || (axisMask[0] || axisMask[2])) axisMask = { 0, 1, 0, 0 };
            else axisMask = { 0, 1, 0, !axisMask[3] };
        }
        if (Input::KeyPressed(GLFW_KEY_Z)) {
            if (!axisMask[2] || (axisMask[0] || axisMask[1])) axisMask = { 0, 0, 1, 0 };
            else axisMask = { 0, 0, 1, !axisMask[3] };
        }
    }

    void SelectAxes()
    {
        if (Input::KeyDown(GLFW_KEY_LEFT_SHIFT))
        {
            if (Input::KeyPressed(GLFW_KEY_X)) axisMask = { !axisMask[0],  axisMask[1],  axisMask[2], 0 };
            if (Input::KeyPressed(GLFW_KEY_Y)) axisMask = {  axisMask[0], !axisMask[1],  axisMask[2], 0 };
            if (Input::KeyPressed(GLFW_KEY_Z)) axisMask = {  axisMask[0],  axisMask[1], !axisMask[2], 0 };
        }
        else SelectAxis();
    }

    glm::vec3 PositionFromViewRay(glm::vec3 objectStartPosition, float dist)
    {
        float w = Engine::GetWindowSize().x;
        float h = Engine::GetWindowSize().y;

        mx_ += Input::GetMouseDeltaX();
        my_ -= Input::GetMouseDeltaY();

        glm::vec4 viewport = {0, 0, w, h};

        // Get the world-space ray corresponding to the mouse
        glm::vec3 rayOrigin = glm::unProject({mx_, h - my, 0.0f}, AM::ViewMat4, AM::ProjMat4, viewport);
        glm::vec3 rayTarget = glm::unProject({mx_, h - my, 1.0f}, AM::ViewMat4, AM::ProjMat4, viewport);
        glm::vec3 rayDir    = glm::normalize(rayTarget - rayOrigin);

        // Camera position
        glm::vec3 axisOrigin = objectStartPosition;

        glm::vec3 normal = glm::vec3(AM::EditorCam.Front);
        normal = glm::normalize(normal);

        // Ray-plane intersection
        float denom = glm::dot(normal, rayDir);
        if (fabs(denom) < 0.0001f) return axisOrigin; // parallel

        float t = glm::dot(normal, axisOrigin - rayOrigin) / denom;
        return rayOrigin + rayDir * t;
    }

    glm::vec3 CalculatePointOnAxisOrPlane(glm::vec3 objectStartPosition, float dist)
    {
        float w  = Engine::GetWindowSize().x;
        float h  = Engine::GetWindowSize().y;

        mx += Input::GetMouseDeltaX();
        my -= Input::GetMouseDeltaY();

        glm::vec4 viewport = {0, 0, w, h};

        // Get world-space ray from mouse
        glm::vec3 rayOrigin = glm::unProject({mx, h - my, 0.0f}, AM::ViewMat4, AM::ProjMat4, viewport);
        glm::vec3 rayTarget = glm::unProject({mx, h - my, 1.0f}, AM::ViewMat4, AM::ProjMat4, viewport);
        glm::vec3 rayDir    = glm::normalize(rayTarget - rayOrigin);

        if (axisMask[0] && axisMask[1] && axisMask[2]) return PositionFromViewRay(objectStartPosition, dist);

        int axisCount = int(axisMask[0]) + int(axisMask[1]) + int(axisMask[2]);
        glm::vec3 axisOrigin = objectStartPosition; // Where drag started

        // --- Case 2: Single axis → project ray onto axis (closest point)
        if (axisCount == 1)
        {
            glm::vec3 axisDir;
            if (axisMask[3]) {
                glm::quat prevRotQuat = glm::quat(glm::radians(previousRot));
                glm::mat3 rotMat = glm::mat3_cast(prevRotQuat);

                if      (axisMask[0]) axisDir = rotMat[0];
                else if (axisMask[1]) axisDir = rotMat[1];
                else if (axisMask[2]) axisDir = rotMat[2];
            }
            else axisDir = glm::vec3(axisMask[0], axisMask[1], axisMask[2]);

            glm::vec3 w0 = rayOrigin - axisOrigin;
            float a = glm::dot(rayDir, rayDir); // =1
            float b = glm::dot(rayDir, axisDir);
            float c = glm::dot(axisDir, axisDir); // =1
            float d = glm::dot(rayDir, w0);
            float e = glm::dot(axisDir, w0);

            float denom = a * c - b * b;
            if (fabs(denom) < 0.0001f) return axisOrigin;

            float tAxis = (a * e - b * d) / denom;
            return axisOrigin + axisDir * tAxis;
        }

        // --- Case 3: Two axes → project onto plane defined by those axes
        // Plane normal is the axis we’re *not* using
        glm::vec3 normal = glm::vec3(!axisMask[0], !axisMask[1], !axisMask[2]);
        normal = glm::normalize(normal);

        // Ray-plane intersection
        float denom = glm::dot(normal, rayDir);
        if (fabs(denom) < 0.0001f) return axisOrigin; // parallel

        float t = glm::dot(normal, axisOrigin - rayOrigin) / denom;
        return rayOrigin + rayDir * t;
    }

    void Transform()
    {
        if ((Input::GetInputContext() == Input::Game ||
             Input::GetInputContext() == Input::PopupMenu ||
             Input::GetInputContext() == Input::Transforming) && !SM::SceneNodes.empty())
        {
            SM::SceneNode* node = SM::SceneNodes[SM::GetSelectedIndex()];

            if (Input::KeyDown(GLFW_KEY_LEFT_ALT)) {
                if (node->GetType() != SM::NodeType::Object_) return;

                SM::Object* object = dynamic_cast<SM::Object*>(node);
                if (Input::KeyPressed(GLFW_KEY_R)) object->SetRotationEuler({});
                if (Input::KeyPressed(GLFW_KEY_G)) object->SetPosition({});
                if (Input::KeyPressed(GLFW_KEY_T)) object->SetScale(glm::vec3(1.0f));
            }
            if (Input::KeyPressed(GLFW_KEY_G) && !AM::EditorCam.Moving && !AM::EditorCam.Turning)
            {
                axisMask = { 1, 1, 1, 0 };

                translating = !translating;
                rotating    = false;
                scaling     = false;
                previousPos = node->GetPosition();
                // previousRot = node->GetRotationEuler();
                dist = glm::length(AM::EditorCam.Position - node->GetPosition());

                glm::vec2 startPosScreen = qk::WorldToScreen(previousPos);
                mx  = startPosScreen.x;
                my  = Engine::GetWindowSize().y - startPosScreen.y;
                mx_ = startPosScreen.x;
                my_ = Engine::GetWindowSize().y - startPosScreen.y;

                Input::SetInputContext(Input::Transforming);
            }
            if (Input::KeyPressed(GLFW_KEY_R) && !AM::EditorCam.Moving && !AM::EditorCam.Turning)
            {
                if (node->GetType() != SM::NodeType::Object_) return;
                SM::Object* object = dynamic_cast<SM::Object*>(node);

                axisMask = { 1, 1, 1, 0 };

                rotating    = !rotating;
                translating = false;
                scaling     = false;
                previousPos = object->GetPosition();
                previousRot = object->GetRotationEuler();

                glm::vec2 screen_pos = qk::WorldToScreen(previousPos);
                glm::vec2 mouse_pos  = glm::vec2(Input::GetMouseX(), Engine::GetWindowSize().y - Input::GetMouseY());
                glm::vec2 neutral = mouse_pos - screen_pos;
                
                startAngle = glm::degrees((std::atan2f(neutral.x, neutral.y)));
                rs_delta = 0.0f;

                Input::SetInputContext(Input::Transforming);
            }
            if (Input::KeyPressed(GLFW_KEY_T) && !AM::EditorCam.Moving && !AM::EditorCam.Turning)
            {
                if (node->GetType() != SM::NodeType::Object_) return;
                SM::Object* object = dynamic_cast<SM::Object*>(node);

                axisMask = { 1, 1, 1, 1 };

                rotating    = false;
                translating = false;
                scaling     = !scaling;
                previousPos = object->GetPosition();
                previousRot = object->GetRotationEuler();
                previousScale = object->GetScale();

                glm::vec2 screen_pos = qk::WorldToScreen(previousPos);

                rs_delta = 0.0f;

                Input::SetInputContext(Input::Transforming);
            }
            if (translating)
            {
                SelectAxes();
                glm::vec3 new_pos = CalculatePointOnAxisOrPlane(previousPos, dist);
                node->SetPosition(new_pos);

                if (Input::MouseButtonPressed(GLFW_MOUSE_BUTTON_1) ||
                    AM::EditorCam.Turning ||
                    AM::EditorCam.Moving)
                {
                    translating = false;
                    axisMask = { 1, 1, 1, 0 };
                    Input::SetInputContext(Input::Game);
                    Input::ConsumeMouseButton(GLFW_MOUSE_BUTTON_1);
                }
                else if (Input::KeyPressed(GLFW_KEY_ESCAPE)) {
                    translating = false;
                    axisMask = { 1, 1, 1, 0 };
                    node->SetPosition(previousPos);
                    Input::SetInputContext(Input::Game);
                }
            }
            if (rotating)
            {
                SelectAxis();

                int axisCount = int(axisMask[0]) + int(axisMask[1]) + int(axisMask[2]);

                glm::quat prevRotQuat = glm::quat(glm::radians(previousRot));
                              
                glm::vec2 screen_pos = qk::WorldToScreen(previousPos);
                glm::vec2 mouse_pos  = glm::vec2(Input::GetMouseX(), Engine::GetWindowSize().y - Input::GetMouseY());
                glm::vec2 neutral = mouse_pos - screen_pos;

                if (axisCount == 3) angle = startAngle - glm::degrees(std::atan2f(neutral.x, neutral.y));
                else {
                    // Move cursor if its reaching right edge
                    if (Input::GetMouseX() >= (float)Engine::GetWindowSize().x - 5)
                        glfwSetCursorPos(Engine::WindowPtr(), 10, Input::GetMouseY());
                    // Move cursor if its at the left
                    else if (Input::GetMouseX() <= 5)
                        glfwSetCursorPos(Engine::WindowPtr(), Engine::GetWindowSize().x - 10, Input::GetMouseY());

                    rs_delta += Input::GetMouseDeltaX();
                    angle = (rs_delta / Engine::GetWindowSize().x) * 720.0f;
                }

                glm::vec3 front = AM::EditorCam.Front;
                glm::vec3 dir;

                if (axisMask[3]) {
                    glm::mat3 rotMat = glm::mat3_cast(prevRotQuat);

                    if      (axisMask[0]) dir = rotMat[0];
                    else if (axisMask[1]) dir = rotMat[1];
                    else if (axisMask[2]) dir = rotMat[2];
                }
                else dir = glm::vec3(axisMask[0], axisMask[1], -axisMask[2]);

                glm::vec3 axis = (axisCount == 3) ? front : dir;
                glm::quat delta = glm::angleAxis(glm::radians(-angle), glm::normalize(axis));

                SM::Object* object = dynamic_cast<SM::Object*>(node);
                object->SetRotationQuat(delta * prevRotQuat);

                if (Input::MouseButtonPressed(GLFW_MOUSE_BUTTON_1) ||
                    AM::EditorCam.Turning ||
                    AM::EditorCam.Moving)
                {
                    rotating = false;
                    axisMask = { 1, 1, 1, 0 };
                    Input::SetInputContext(Input::Game);
                    Input::ConsumeMouseButton(GLFW_MOUSE_BUTTON_1);
                }
                else if (Input::KeyPressed(GLFW_KEY_ESCAPE))
                {
                    rotating = false;
                    axisMask = { 1, 1, 1, 0 };
                    rs_delta  = 0.0f;
                    object->SetRotationEuler(previousRot);
                    Input::SetInputContext(Input::Game);
                }
            }
            if (scaling)
            {
                SelectAxes();

                rs_delta += Input::GetMouseDeltaX();
                float scale_factor = 1.0f;
                scale_factor += (rs_delta / Engine::GetWindowSize().y) * 4.0f;

                // Move cursor if its reaching right
                if (Input::GetMouseX() >= (float)Engine::GetWindowSize().x - 5) {
                    glfwSetCursorPos(Engine::WindowPtr(), 10, Input::GetMouseY());
                    rs_delta += (float)Engine::GetWindowSize().x;
                }
                // Move cursor if its at the left
                else if (Input::GetMouseX() <= 5) {
                    glfwSetCursorPos(Engine::WindowPtr(), Engine::GetWindowSize().x - 10, Input::GetMouseY());
                    rs_delta -= (float)Engine::GetWindowSize().x;
                }

                int axisCount = int(axisMask[0]) + int(axisMask[1]) + int(axisMask[2]);
                glm::vec3 axis;
                glm::vec3 new_scale;

                new_scale = glm::vec3(
                    axisMask[0] ? previousScale.x * scale_factor : previousScale.x,
                    axisMask[1] ? previousScale.y * scale_factor : previousScale.y,
                    axisMask[2] ? previousScale.z * scale_factor : previousScale.z
                );

                SM::Object* object = dynamic_cast<SM::Object*>(node);
                object->SetScale(new_scale);

                if (Input::MouseButtonPressed(GLFW_MOUSE_BUTTON_1) ||
                    AM::EditorCam.Turning ||
                    AM::EditorCam.Moving)
                {
                    scaling = false;
                    axisMask = { 1, 1, 1, 0 };
                    Input::SetInputContext(Input::Game);
                    Input::ConsumeMouseButton(GLFW_MOUSE_BUTTON_1);
                }
                else if (Input::KeyPressed(GLFW_KEY_ESCAPE))
                {
                    scaling = false;
                    axisMask = { 1, 1, 1, 0 };
                    rs_delta  = 0.0f;
                    object->SetScale(previousScale);
                    Input::SetInputContext(Input::Game);
                }
            }
        }
    }

    float axisWidth      = 0.01f;
    float axisLengthBase = 10.0f;
    void DrawAxis()
    {
        if (SM::SceneNodes.empty()) return;
        SM::SceneNode* node = SM::SceneNodes[SM::GetSelectedIndex()];

        if ((translating || rotating || scaling) && Input::GetInputContext() == Input::Transforming)
        {
            if (axisMask[0] && axisMask[1] && axisMask[2]) return;

            float axisLength = axisLengthBase * glm::length(AM::EditorCam.Position - node->GetPosition());

            glm::mat4 matrix;
            if (node->GetType() == SM::NodeType::Object_) {
                SM::Object* object = dynamic_cast<SM::Object*>(node);
                matrix = object->GetModelMatrix();
            }
            else matrix = glm::mat4(1.0f);

            for (int i = 0; i < 3; ++i) {
                glm::vec3 newScale;
                if (axisMask[0]) newScale = glm::vec3(axisLength, axisWidth,  axisWidth);
                if (axisMask[1]) newScale = glm::vec3(axisWidth,  axisLength, axisWidth);
                if (axisMask[2]) newScale = glm::vec3(axisWidth,  axisWidth,  axisLength);
                matrix[i] = glm::vec4(glm::normalize(glm::vec3(matrix[i])) * newScale[i], 0.0f);
            }

            glDisable(GL_DEPTH_TEST);
            if (axisMask[0])
            {
                if (axisMask[3]) qk::DrawDebugCubeMatrix(matrix, glm::vec3(1, 0, 0));
                else qk::DrawDebugCube(node->GetPosition(),
                                       glm::vec3(axisLength, axisWidth, axisWidth),
                                       glm::vec3(1, 0, 0));
            }
            if (axisMask[1])
            {
                if (axisMask[3]) qk::DrawDebugCubeMatrix(matrix, glm::vec3(0, 1, 0));
                else qk::DrawDebugCube(node->GetPosition(),
                                       glm::vec3(axisWidth, axisLength, axisWidth),
                                       glm::vec3(0, 1, 0));
            }
            if (axisMask[2])
            {
                if (axisMask[3]) qk::DrawDebugCubeMatrix(matrix, glm::vec3(0, 0, 1));
                else qk::DrawDebugCube(node->GetPosition(),
                                       glm::vec3(axisWidth, axisWidth, axisLength),
                                       glm::vec3(0, 0, 1));
            }
            glEnable(GL_DEPTH_TEST);
        }
    }

    void DrawAxisText()
    {
        if ((translating || rotating || scaling) && Input::GetInputContext() == Input::Transforming) {
            Text::RenderCenteredBG(axisFromMask(), Engine::GetWindowSize().x / 2, Engine::GetWindowSize().y - 40, 0.5f, glm::vec3(0.95f), glm::vec3(0.0f));
            if (rotating) Text::RenderCenteredBG(std::format("{:.{}f}", angle, 2), Engine::GetWindowSize().x / 2, Engine::GetWindowSize().y - 60, 0.5f, glm::vec3(0.95f), glm::vec3(0.0f));
        }
    }

    void Initialize()
    {
        Engine::RegisterEditorFunction(Transform);
        Engine::RegisterEditorDraw3DFunction(DrawAxis);
        Engine::RegisterEditorDrawUIFunction(DrawAxisText);
    }
}
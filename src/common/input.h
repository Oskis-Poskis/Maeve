#pragma once

#include <glm/glm.hpp>

namespace Input
{
    enum InputContext
    {
        Game,
        Menu,
    };

    std::string InputContextString();

    void SetInputContext(InputContext Context);
    InputContext GetInputContext();
    InputContext GetPreviousInputContext();

    void Initialize();
    void Update();
    bool KeyPressed(unsigned int keycode);
    bool KeyDown(unsigned int keycode);
    void ResetKeyPress(unsigned int keycode);
    bool LeftMBDown();
    bool RightMBDown();
    bool MiddleMBDown();

    glm::vec2 GetMouseXY();
    float GetMouseX();
    float GetMouseY();
    float GetMouseDeltaX();
    float GetMouseDeltaY();
}
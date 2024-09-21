#pragma once

namespace Input
{
    void Initialize();
    void Update();
    bool KeyPressed(unsigned int keycode);
    bool KeyDown(unsigned int keycode);
    bool LeftMBDown();
    bool RightMBDown();

    float GetMouseX();
    float GetMouseY();
    float GetMouseDeltaX();
    float GetMouseDeltaY();
}
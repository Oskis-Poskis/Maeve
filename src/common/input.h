#pragma once

#include <glm/glm.hpp>

namespace Input
{
    enum InputContext
    {
        Game,
        Menu,
        PopupMenu,
        TextInput,
        Transforming
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

    bool MouseButtonReleased(int button);
    bool MouseButtonPressed(int button);
    bool MouseButtonDown(int button);
    void ConsumeMouseButton(int button);
    std::string GetTypedCharacters(bool OnlyNumerals = false, bool OnlyIntegers = false);

    glm::vec2 GetMouseXY();
    float GetMouseX();
    float GetMouseY();
    float GetMouseDeltaX();
    float GetMouseDeltaY();
}
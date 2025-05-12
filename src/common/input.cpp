#include <iostream>

#include <GLFW/glfw3.h>

#include "input.h"
#include "stat_counter.h"
#include "../engine/render_engine.h"

namespace Input
{
    bool _keyPressed[348];
    bool _keyDown[348];
    bool _keyDownLastFrame[348];

    bool _mousePressed[8];
    bool _mouseDown[8];
    bool _mouseDownLastFrame[8];
    bool _mouseConsumed[8]; 

    double _mouseX, _mouseY;
    float  _xDelta, _yDelta;
    float  _lastX,  _lastY;

    InputContext inputContext = Game;
    InputContext previousInputContext;

    std::string g_TypedCharacters;
    bool g_TextInputActive    = false;
    bool justEnabledTextInput = false;
    void CharCallback(GLFWwindow* window, unsigned int codepoint)
    {
        if (!g_TextInputActive) return;
        if (justEnabledTextInput) {
            justEnabledTextInput = false;
            return;
        }

        if (codepoint >= 32 && codepoint <= 126) { // Printable ASCII
            g_TypedCharacters += static_cast<char>(codepoint);
        }
    }

    void Initialize()
    {
        _lastX = Engine::GetWindowSize().x / 2;
        _lastY = Engine::GetWindowSize().y / 2;

        glfwSetCharCallback(Engine::WindowPtr(), CharCallback);
    }

    float timer = 0;
    void Update()
    {
        if (timer >= (1 / 60.0f))
        {
            for (int i = 32; i < 348; i++)
            {
                // Key Down
                if (glfwGetKey(Engine::WindowPtr(), i) == GLFW_PRESS) _keyDown[i] = true;
                else _keyDown[i] = false;

                // Single Press
                if (_keyDown[i] && !_keyDownLastFrame[i]) _keyPressed[i] = true;
                else _keyPressed[i] = false;

                _keyDownLastFrame[i] = _keyDown[i];
            }

            for (int i = 0; i < 8; i++)
            {
                // Button Down
                if (glfwGetMouseButton(Engine::WindowPtr(), i) == GLFW_PRESS)
                    _mouseDown[i] = true;
                else
                    _mouseDown[i] = false;

                // Single Press (frame perfect)
                if (_mouseDown[i] && !_mouseDownLastFrame[i])
                    _mousePressed[i] = true;
                else
                    _mousePressed[i] = false;

                // Save for next frame
                _mouseDownLastFrame[i] = _mouseDown[i];

                // Reset consumed
                _mouseConsumed[i] = false;
            }

            glfwGetCursorPos(Engine::WindowPtr(), &_mouseX, &_mouseY);

            _xDelta = _mouseX - _lastX;
            _yDelta = _lastY - _mouseY;
            _lastX = _mouseX;
            _lastY = _mouseY;
        }
        timer += Stats::GetDeltaTime();
    }

    std::string InputContextString()
    {
        switch (inputContext)
        {
            case Game:         return "Game";
            case Menu:         return "Menu";
            case TextInput:    return "Text Input";
            case PopupMenu:    return "Popup Menu";
            case Transforming: return "Transforming";
            default:           return "This should not happen";
        }
    }

    void SetInputContext(InputContext Context)
    {
        previousInputContext = inputContext;
        inputContext = Context;
    }

    InputContext GetInputContext()
    {
        return inputContext;
    }

    InputContext GetPreviousInputContext()
    {
        return previousInputContext;
    }

    bool KeyPressed(unsigned int keycode)
    {
        return _keyPressed[keycode];
    }
    
    bool KeyDown(unsigned int keycode)
    {
        return _keyDown[keycode];
    }

    void ResetKeyPress(unsigned int keycode)
    {
        _keyPressed[keycode] = false;
    }

    bool MouseButtonPressed(int button)
    {
        return _mousePressed[button] && !_mouseConsumed[button];
    }

    bool MouseButtonDown(int button)
    {
        return _mouseDown[button] && !_mouseConsumed[button];
    }

    bool MouseButtonReleased(int button)
    {
        return _mouseDownLastFrame[button] && _mouseDown[button];
    }

    void ConsumeMouseButton(int button)
    {
        _mouseConsumed[button] = true;
    }

    std::string GetTypedCharacters(bool OnlyNumbers, bool OnlyIntegers)
    {
        std::string result = "";

        for (char c : g_TypedCharacters)
        {
            if (OnlyNumbers)
            {
                if (c >= '0' && c <= '9')
                    result += c;
                else if (c == '-' || c == '+' || (!OnlyIntegers && c == '.'))
                    result += c;
            }
            else result += c;
        }

        g_TypedCharacters.clear();

        return result;
    }

    void ClearCharList() { g_TypedCharacters.clear(); }

    void EnableTextInput(bool skipFirstInput)
    {
        ClearCharList();
        g_TextInputActive = true;
        justEnabledTextInput = skipFirstInput;
    }

    void DisableTextInput()
    {
        ClearCharList();
        g_TextInputActive = false;
    }

    glm::vec2 GetMouseXY()
    {
        return glm::vec2(_mouseX, _mouseY);
    }

    float GetMouseX()
    {
        return _mouseX;
    }

    float GetMouseY()
    {
        return _mouseY;
    }

    float GetMouseDeltaX()
    {
        return _xDelta;
    }
    
    float GetMouseDeltaY()
    {
        return _yDelta;
    }
}
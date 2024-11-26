#include <iostream>

#include "input.h"
#include "stat_counter.h"
#include "../engine/render_engine.h"

namespace Input
{
    bool _keyPressed[348];
    bool _keyDown[348];
    bool _keyDownLastFrame[348];

    double _mouseX, _mouseY;
    float  _xDelta, _yDelta;
    float  _lastX,  _lastY;

    InputContext inputContext = Game;
    InputContext previousInputContext;

    void Initialize()
    {
        _lastX = Engine::GetWindowSize().x / 2;
        _lastY = Engine::GetWindowSize().y / 2;
    }

    float timer = 0;
    void Update()
    {
        if (timer >= (1 / 60.0f))
        {
            for (int i = 32; i < 348; i++)
            {
                // Key Down
                if (glfwGetKey(Engine::GetWindowPointer(), i) == GLFW_PRESS) _keyDown[i] = true;
                else _keyDown[i] = false;

                // Single Press
                if (_keyDown[i] && !_keyDownLastFrame[i]) _keyPressed[i] = true;
                else _keyPressed[i] = false;

                _keyDownLastFrame[i] = _keyDown[i];
            }

            glfwGetCursorPos(Engine::GetWindowPointer(), &_mouseX, &_mouseY);

            _xDelta = _mouseX - _lastX;
            _yDelta = _lastY - _mouseY;
            _lastX = _mouseX;
            _lastY = _mouseY;
        }
        timer += Stats::GetDeltaTime();
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

    bool RightMBDown()
    {
        return glfwGetMouseButton(Engine::GetWindowPointer(), GLFW_MOUSE_BUTTON_2) == GLFW_PRESS ? 1 : 0;
    }

    bool LeftMBDown()
    {
        return glfwGetMouseButton(Engine::GetWindowPointer(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS ? 1 : 0;
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
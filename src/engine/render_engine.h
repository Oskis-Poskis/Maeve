#pragma once

#include <functional>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Engine
{
    // Window
    GLFWwindow* GetWindowPointer();
    glm::ivec2 GetWindowSize();
    glm::ivec2 GetMonitorSize();
    void ToggleFullscreen();

    void RegisterResizeCallback(const std::function<void(int, int)>& callback);
    void RegisterEditorFunction(const std::function<void()>& func);

    // Running
    void Initialize();
    void NewFrame();
    void Run();
    void Quit();
};
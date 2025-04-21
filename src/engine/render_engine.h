#pragma once

#include <functional>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace Engine
{
    // Window
    GLFWwindow* WindowPtr();
    glm::ivec2 GetWindowSize();
    glm::ivec2 GetMonitorSize();
    void ToggleFullscreen();

    void RegisterResizeCallback(const std::function<void(int, int)>& callback);
    void RegisterEditorFunction(const std::function<void()>& func);
    void RegisterEditorDraw3DFunction(const std::function<void()>& func);
    void RegisterEditorDrawUIFunction(const std::function<void()>& func);

    // Running
    void Initialize();
    void NewFrame();
    void Run();
    void Quit();
};
#pragma once

#include <functional>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace Engine
{
    enum DebugMode {
        None      = 1,
        BVH       = 2,
        Deferred  = 3,
        Stats     = 4,
        ShadowMap = 5,
    };
    inline DebugMode debugMode = DebugMode::None;
    inline const char* DebugModeToString(DebugMode mode) {
        switch (mode) {
            case None:      return "None";
            case BVH:       return "BVH";
            case Deferred:  return "Deferred";
            case Stats:     return "Stats";
            case ShadowMap: return "Shadow Map";

            default:   return "Unknown";
        }
    }

    // Window
    GLFWwindow* WindowPtr();
    glm::ivec2 GetWindowSize();
    glm::ivec2 GetMonitorSize();
    float GetWindowAspect();
    void ToggleFullscreen();

    void RegisterResizeCallback(const std::function<void(int, int)>& callback);
    void RegisterEditorFunction(const std::function<void()>& func);
    void RegisterEditorDraw3DFunction(const std::function<void()>& func);
    void RegisterEditorDrawUIFunction(const std::function<void()>& func);
    void RegisterEditorReloadShadersFunction(const std::function<void()>& func);

    // Running
    void Initialize();
    void NewFrame();
    void Run();
    void Quit();
};
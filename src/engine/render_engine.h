#pragma once

#include "../../lib/glm/glm.hpp"
#include "../../lib/glad/glad.h"
#include <GLFW/glfw3.h>

namespace Engine
{
    // Window
    GLFWwindow* GetWindowPointer();
    glm::ivec2 GetWindowSize();
    glm::ivec2 GetMonitorSize();
    void ToggleFullscreen();

    // Running
    void Initialize();
    void NewFrame(float fov, glm::mat4 vMat, glm::vec3 cPos);
    void Quit();
};
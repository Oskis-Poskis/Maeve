#include <iostream>
#include <filesystem>

#include "stat_counter.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glext.h>

namespace Statistics
{
    float _fps = 0.0f;
    float _ms  = 0.0f;
    float _deltaTime = 0.0f;
    float _lastFrame = 0.0f;
    std::string _vendor = "idk man";

    void Count(float time)
    {
        float currentFrame = time;
        _deltaTime = currentFrame - _lastFrame;
        _lastFrame = currentFrame;

        _fps = (1.0f / _deltaTime);
        _ms  = (1000.0f / (1.0f / _deltaTime));
    }

    void Initialize()
    {
        Renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        Vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        ProjectPath = std::filesystem::current_path().string();
    }

    float GetFPS()
    {
        return _fps;
    }

    float GetMS()
    {
        return _ms;
    }

    float GetDeltaTime()
    {
        return _deltaTime;
    }

    int GetVramUsageMb()
    {
        int totalMemoryKb;
        int currentMemoryKb;
        if (Vendor == "NVIDIA Corporation")
        {
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKb);
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentMemoryKb);

            int usedMemoryKb = totalMemoryKb - currentMemoryKb;
            return usedMemoryKb / 1024;
        }
        else
        {
            int vboMemory[4];
            glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, vboMemory);
            int usedMemoryKb = totalMemoryKb - vboMemory[2];
            return usedMemoryKb / 1024;
            return 0;
        }
    }
    
    int GetVRAMTotalMb()
    {
        int totalMemoryKb;
        glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKb);
        return totalMemoryKb / 1024;
    }
}
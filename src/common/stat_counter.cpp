#include <iostream>
#include <filesystem>
#include <format>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glext.h>

#include "stat_counter.h"
#include "../engine/render_engine.h"
#include "../engine/asset_manager.h"
#include "../engine/scene_manager.h"
#include "../ui/text_renderer.h"
#include "../common/qk.h"
#include "../common/input.h"

namespace Stats
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

    float yOffset       = 26;
    float textScaling   = 0.5f;
    float timer         = 0;
    std::string FPS, ms = "";

    float avgfps = 60.0f;
    float avgms  = 0.016f;
    float alpha  = 0.95f;
    void DrawStats()
    {
        std::string memory = std::format("VRAM: {} / {}mb", Stats::GetVramUsageMb(), Stats::GetVRAMTotalMb());
        timer += Stats::GetDeltaTime();
        if (timer >= (1 / (20.0f)))
        {
            timer = 0.0f;
            
            avgfps = alpha * avgfps + (1.0f - alpha) * GetFPS();
            avgms  = alpha * avgms + (1.0f - alpha) * GetMS();

            FPS = std::format("{:<4} {:>7.2f}", "FPS:", avgfps);
            ms  = std::format("{:<4} {:>7.2f}", "ms:",  avgms);
        }

        std::string meshes = std::format<int>("Meshes in memory: {} ({} triangles)", AssetManager::Meshes.size(), qk::FmtK(AssetManager::UniqueMeshTriCount));
        std::string objects = std::format<int>("Objects in scene: {} ({} triangles)", SceneManager::Objects.size(), qk::FmtK(SceneManager::ObjectsTriCount));
        
        glDisable(GL_DEPTH_TEST);
        float lineSpacing = 20 * Text::GetGlobalTextScaling();
        Text::Render(Stats::Renderer,                                          15, Engine::GetWindowSize().y - yOffset - 0 * lineSpacing, textScaling);
        Text::Render("Window Size: " + qk::FormatVec(Engine::GetWindowSize()), 15, Engine::GetWindowSize().y - yOffset - 1 * lineSpacing, textScaling);
        Text::Render(FPS,                                                      15, Engine::GetWindowSize().y - yOffset - 3 * lineSpacing, textScaling);
        Text::Render(ms,                                                       15, Engine::GetWindowSize().y - yOffset - 4 * lineSpacing, textScaling);
        if (Vendor == "NVIDIA Corporation") Text::Render(memory,               15, Engine::GetWindowSize().y - yOffset - 5 * lineSpacing, textScaling);
        Text::Render(meshes,                                                   15, Engine::GetWindowSize().y - yOffset - 7 * lineSpacing, textScaling);
        Text::Render(objects,                                                  15, Engine::GetWindowSize().y - yOffset - 8 * lineSpacing, textScaling);
        Text::Render("Input Context: " + Input::InputContextString(),          15, Engine::GetWindowSize().y - yOffset - 10 * lineSpacing, textScaling);
        glEnable(GL_DEPTH_TEST);
    }

    void Initialize()
    {
        Renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        Vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
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
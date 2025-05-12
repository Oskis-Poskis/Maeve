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
        std::string memory = std::format("VRAM: {} / {}mb", GetVRAMUsageMb(), GetVRAMTotalMb());
        timer += Stats::GetDeltaTime();
        if (timer >= (1 / (60.0f)))
        {
            timer = 0.0f;
            
            avgfps = alpha * avgfps + (1.0f - alpha) * GetFPS();
            avgms  = alpha * avgms  + (1.0f - alpha) * GetMS();

            FPS = qk::LabelWithPaddedNumber("FPS: ", avgfps, 5, 7);
            ms  = qk::LabelWithPaddedNumber("ms: ",  avgms,  5, 7);
        }

        std::string meshes  = std::format<int>("Meshes in memory: {} ({} triangles)", AM::Meshes.size(), qk::FmtK(AM::UniqueMeshTriCount));
        std::string objects = std::format<int>("Nodes in scene: {} ({} triangles)", SM::SceneNodes.size(), qk::FmtK(SM::ObjectsTriCount));
        
        glDisable(GL_DEPTH_TEST);
        float lineSpacing = 20 * Text::GetGlobalTextScaling();
        Text::Render(Stats::Renderer,                                                  15, Engine::GetWindowSize().y - yOffset - 0  * lineSpacing, textScaling);
        Text::Render("Window Size: " + qk::FormatVec(Engine::GetWindowSize()),         15, Engine::GetWindowSize().y - yOffset - 1  * lineSpacing, textScaling);
        Text::Render(FPS,                                                              15, Engine::GetWindowSize().y - yOffset - 3  * lineSpacing, textScaling);
        Text::Render(ms,                                                               15, Engine::GetWindowSize().y - yOffset - 4  * lineSpacing, textScaling);
        Text::Render(memory,                                                           15, Engine::GetWindowSize().y - yOffset - 5  * lineSpacing, textScaling);
        Text::Render(meshes,                                                           15, Engine::GetWindowSize().y - yOffset - 7  * lineSpacing, textScaling);
        Text::Render(objects,                                                          15, Engine::GetWindowSize().y - yOffset - 8  * lineSpacing, textScaling);
        Text::Render("Input Context: " + Input::InputContextString(),                  15, Engine::GetWindowSize().y - yOffset - 10 * lineSpacing, textScaling);
        Text::Render("Debug Mode: "    + std::string(Engine::DebugModeToString(Engine::debugMode)), 15, Engine::GetWindowSize().y - yOffset - 11 * lineSpacing, textScaling);
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

    bool HasExtension(const char* extName) {
        GLint nExt;
        glGetIntegerv(GL_NUM_EXTENSIONS, &nExt);
        for (GLint i = 0; i < nExt; ++i) {
            if (std::string(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i))) == extName)
                return true;
        }
        return false;
    }

    // Returns VRAM usage in MB, or -1 if unsupported
    int GetVRAMUsageMb()
    {
        const char* vendorStr = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        std::string vendor = vendorStr ? vendorStr : "";

        // NVIDIA (NVX extension)
        if (vendor.find("NVIDIA") != std::string::npos && HasExtension("GL_NVX_gpu_memory_info")) {
            GLint totalKb = 0, curKb = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalKb);
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &curKb);
            return (totalKb - curKb) / 1024;  // Usage in MB
        }

        return -1; // Unknown vendor or unsupported
    }

    // Returns total VRAM in MB, or -1 if unsupported
    int GetVRAMTotalMb()
    {
        const char* vendorStr = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        std::string vendor = vendorStr ? vendorStr : "";

        if (vendor.find("NVIDIA") != std::string::npos && HasExtension("GL_NVX_gpu_memory_info")) {
            GLint totalKb = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalKb);
            return totalKb / 1024; // in MB
        }

        return -1; // Unknown vendor or unsupported
    }
}
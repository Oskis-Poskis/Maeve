#pragma once

#include <string>

namespace Stats
{
    inline std::string Vendor;
    inline std::string Renderer;
    inline std::string ProjectPath;
    void Initialize();
    float GetFPS();
    float GetMS();
    float GetDeltaTime();
    int GetVramUsageMb();
    int GetVRAMTotalMb();
    
    void Count(float time);
    void DrawStats();
}
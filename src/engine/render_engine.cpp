#include <iostream>
#include <iomanip>
#include <string>
#include <format>
#include <random>

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "render_engine.h"
#include "asset_manager.h"
#include "scene_manager.h"
#include "deferred/deffered_manager.h"
#include "editor/object_manipulation.h"
#include "editor/light_manipulation.h"
#include "../common/stat_counter.h"
#include "../common/input.h"
#include "../common/qk.h"
#include "../ui/ui.h"
#include "../ui/text_renderer.h"
#include "../ui/box_ui.h"

namespace Engine
{
    GLFWwindow* window = NULL;
    int windowWidth    = 1920;
    int windowHeight   = 1080;
    int monitorWidth   = 0;
    int monitorHeight  = 0;

    bool isFullscreen            = false;
    int unfullscreenWindowWidth  = 512;
    int unfullscreenWindowHeight = 512;
    int unfullscreenwindowPosX   = 0;
    int unfullscreenwindowPosY   = 0;

    std::vector<std::function<void(int, int)>> resizeCallbacks;
    std::vector<std::function<void()>> editorEvents;
    std::vector<std::function<void()>> editorDraw3DEvents;
    std::vector<std::function<void()>> editorDrawUIEvents;
    std::vector<std::function<void()>> editorReloadShaderEvents;

    void errorCallback(int error, const char* description);
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void windowResized(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);

        windowWidth  = width;
        windowHeight = height;

        for (const auto& callback : resizeCallbacks) { callback(windowWidth, windowHeight); }    }

    void RegisterResizeCallback(const std::function<void(int, int)> &callback) { resizeCallbacks.push_back(callback); }
    void RegisterEditorFunction(const std::function<void()>& func) { editorEvents.push_back(func); }
    void RegisterEditorDraw3DFunction(const std::function<void()>& func) { editorDraw3DEvents.push_back(func); }
    void RegisterEditorDrawUIFunction(const std::function<void()>& func) { editorDrawUIEvents.push_back(func); }
    void RegisterEditorReloadShadersFunction(const std::function<void()> &func) { editorReloadShaderEvents.push_back(func); }

    void windowMaximized(GLFWwindow* window, int maximized)
    {
        glfwGetWindowSize(window, &windowWidth,& windowHeight);
        windowResized(window, windowWidth, windowHeight);
    }

    void Initialize()
    {
        std::string OS;
        #ifdef _WIN32
            OS = "Windows";
        #elif __linux__
            OS = "Linux";
        #else
            OS = "Unknown";
        #endif

        std::cout << "\n";
        std::cout << "[>] " + std::string(70, '-') + "\n";
        std::cout << std::format("[.] Maeve starting on {}\n", OS);
        std::cout << "[>] " + std::string(70, '-') + "\n\n";

        if (glfwInit())
        {
            std::cout << "[:] Successfully initialized GLFW\n";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        
            window = glfwCreateWindow(windowWidth, windowHeight, "Maeve 0.0.1", NULL, NULL);
            if (!window) std::cout << "[!] Failed to create window\n";
            glfwMakeContextCurrent(window);
            
            glfwSetErrorCallback(errorCallback);
            glfwSetScrollCallback(window, scrollCallback);
            glfwSetFramebufferSizeCallback(window, windowResized);
            glfwSetWindowMaximizeCallback(window, windowMaximized);

            // glfwSetWindowAttrib(window, GLFW_DECORATED, false);
            glfwSwapInterval(0);

            // Center Window
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            monitorWidth  = mode->width;
            monitorHeight = mode->height;
            glfwSetWindowPos(window, monitorWidth / 2 - windowWidth / 2, monitorHeight / 2 - windowHeight / 2);

            // ToggleFullscreen();
        }
        else std::cout << "[:] Couldn't initialize GLFW\n\n";

        if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) std::cout << "[:] Successfully loaded GLAD\n\n";
        else {
            std::cout << "[:] Couldn't load GLAD\n\n";
            glfwTerminate();
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // BG color set in shader
        glLineWidth(2);

        Stats::Initialize();
        Input::Initialize();
        Deferred::Initialize();
        SM::Initialize();
        AM::Initialize();
        ObjectManipulation::Initialize();
        LightManipulation::Initialize();
        Text::Initialize();
        UI::Initialize();
        BUI::Initialize();
        qk::Initialize();

        windowResized(window, windowWidth, windowHeight);

        debugMode = DebugMode::Stats;
    }

    void Run()
    {
        while (!glfwWindowShouldClose(window)) { NewFrame(); }
        Quit();
    }

    float DrawShadows_Timing;
    float CalcShadows_Timing;
    float GBuffers_Timing;
    float Shading_Timing;
    float PostProcess_Timing;
    float UI_Timing;
    
    void NewFrame()
    {
        glfwPollEvents();
        Input::Update();
        
        if (Input::KeyDown(GLFW_KEY_SPACE) && Input::KeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);
        if (Input::KeyPressed(GLFW_KEY_F11)) ToggleFullscreen();
        if (Input::KeyDown(GLFW_KEY_LEFT_CONTROL) && Input::KeyPressed(GLFW_KEY_Q)) Quit();

        /* EDITOR ONLY */ if (Input::KeyPressed(GLFW_KEY_X) && Input::GetInputContext() == Input::Game && SM::SceneNodes.size() > 0)
        {
            SM::SceneNodes.erase(SM::SceneNodes.begin() + SM::GetSelectedIndex());
            SM::SceneNodeNames.erase(SM::SceneNodeNames.begin() + SM::GetSelectedIndex());
            SM::SelectSceneNode(std::max(SM::GetSelectedIndex() - 1, 0));
            SM::UpdateDrawList();
        }
        
        /* EDITOR ONLY */ qk::ExecuteMainThreadTasks();
        /* EDITOR ONLY */ if (Input::KeyPressed(GLFW_KEY_F)) SM::FocusSelection();
        /* EDITOR ONLY */ for (const auto& func : editorEvents) { func(); }
        /* EDITOR ONLY */ if (Input::KeyPressed(GLFW_KEY_HOME)) for (const auto& func : editorReloadShaderEvents) { func(); }
        auto test = 1;
        
        // Make sure this view matrix is from active camera
        // This should happen after editorEvents
        AM::ViewMat4 = AM::EditorCam.GetViewMatrix();
        SM::UpdateInstanceMatrixSSBO();
        
        // GBuffers --------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, Deferred::GetGBufferFBO());
        qk::BeginGPUTimer("GBuffers");
        Deferred::DrawGBuffers();
        /* EDITOR ONLY */ Deferred::DrawMask(); // R8 texture used for outline generation
        float timing1 = qk::EndGPUTimer("GBuffers");
        if (timing1 != 0.0f) {
            GBuffers_Timing = timing1;
        }

        qk::BeginGPUTimer("Draw Shadows");
        glBindFramebuffer(GL_FRAMEBUFFER, Deferred::GetShadowFBO());
        Deferred::DrawShadows();
        float time_DrawShadows = qk::EndGPUTimer("Draw Shadows");
        if (time_DrawShadows != 0.0f) {
            DrawShadows_Timing = time_DrawShadows;
        }

        qk::BeginGPUTimer("Calc Shadows");
        glBindFramebuffer(GL_FRAMEBUFFER, Deferred::GetGBufferFBO());
        Deferred::CalcShadows();
        float time_CalcShadows = qk::EndGPUTimer("Calc Shadows");
        if (time_CalcShadows != 0.0f) {
            CalcShadows_Timing = time_CalcShadows;
        }
        
        // Draw deferred shaded to color attachment 0
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        qk::BeginGPUTimer("Shading");
        Deferred::DoShading();
        float time_Shading = qk::EndGPUTimer("Shading");
        if (time_Shading != 0.0f) {
            Shading_Timing = time_Shading;
        }
        /* EDITOR ONLY */ for (const auto& func : editorDraw3DEvents) { func(); }
        // -------------------------------------------

        // Display -----------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        qk::BeginGPUTimer("Post Process");
        Deferred::DoPostProcessAndDisplay();
        float time_PP = qk::EndGPUTimer("Post Process");
        if (time_PP != 0.0f) {
            PostProcess_Timing = time_PP;
        }
        /* EDITOR ONLY */ if (debugMode == DebugMode::Deferred)  Deferred::VisualizeGBuffers();
        /* EDITOR ONLY */ if (debugMode == DebugMode::ShadowMap) Deferred::VisualizeShadowMap();
        /* EDITOR ONLY */ for (const auto& func : editorDrawUIEvents) { func(); }

        qk::BeginGPUTimer("UI");
        /* EDITOR ONLY */ UI::Render();
        /* EDITOR ONLY */ BUI::DrawTest();
        
        Stats::Count(glfwGetTime());
        SM::CalculateObjectsTriCount();
        if (debugMode == DebugMode::Stats) {
            int y = Engine::GetWindowSize().y / 2;
            Text::Render(qk::LabelWithPaddedNumber("Draw Shadows:", DrawShadows_Timing, 15, 5),    15, y - 26 * 0, 0.5f);
            Text::Render(qk::LabelWithPaddedNumber("Calc Shadows:", CalcShadows_Timing, 15, 5),    15, y - 26 * 1, 0.5f);
            Text::Render(qk::LabelWithPaddedNumber("GBuffers:", GBuffers_Timing, 15, 5),           15, y - 26 * 2, 0.5f);
            Text::Render(qk::LabelWithPaddedNumber("Shading:", Shading_Timing, 15, 5),             15, y - 26 * 3, 0.5f);
            Text::Render(qk::LabelWithPaddedNumber("Post Process:", PostProcess_Timing, 15, 5),    15, y - 26 * 4, 0.5f);
            Text::Render(qk::LabelWithPaddedNumber("UI:", UI_Timing, 15, 5),                       15, y - 26 * 5, 0.5f);
            
            Stats::DrawStats();
        }
        
        float time_UI = qk::EndGPUTimer("UI");
        if (time_UI != 0.0f) {
            UI_Timing = time_UI;
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        // -------------------------------------------

        glfwSwapBuffers(window);
    }

    void Quit()
    {
        glDeleteFramebuffers(1, &Deferred::GetGBufferFBO());
        glfwTerminate();
        exit(0);
    }

    void ToggleFullscreen()
    {
        if (!isFullscreen) {
            glfwMaximizeWindow(window);
            isFullscreen = true;
        }
        else {
            glfwRestoreWindow(window);
            isFullscreen = false;
        }
    }

    GLFWwindow* WindowPtr() { return window; }
    glm::ivec2  GetWindowSize()    { return glm::ivec2(windowWidth, windowHeight); }
    glm::ivec2  GetMonitorSize()   { return glm::ivec2(monitorWidth, monitorHeight); }

    float GetWindowAspect()
    {
        return (float)windowWidth / windowHeight;
    }

    void errorCallback(int error, const char* description) { fprintf(stderr, "Error: %s\n", description); }
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) { }
}
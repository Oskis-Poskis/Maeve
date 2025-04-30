#include <iostream>
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
#include <iomanip>

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

            glfwSetWindowAttrib(window, GLFW_DECORATED, false);
            glfwSwapInterval(1);

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
        qk::Initialize();

        windowResized(window, windowWidth, windowHeight);
    }

    void Run()
    {
        while (!glfwWindowShouldClose(window)) { NewFrame(); }
        Quit();
    }

    const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    void NewFrame()
    {
        glfwPollEvents();
        Input::Update();
        
        if (Input::KeyDown(GLFW_KEY_SPACE) && Input::KeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);
        if (Input::KeyPressed(GLFW_KEY_F11)) ToggleFullscreen();
        if (Input::KeyDown(GLFW_KEY_LEFT_CONTROL) && Input::KeyPressed(GLFW_KEY_Q)) Quit();
        for (int i = 0; i <= 9; ++i) {
            if (Input::KeyPressed(GLFW_KEY_0 + i) && Input::GetInputContext() == Input::Game) {
                debugMode = static_cast<DebugMode>(i);
                break;
            }
        }

        /* EDITOR ONLY */ for (const auto& func : editorEvents) { func(); }
        /* EDITOR ONLY */ if (Input::KeyPressed(GLFW_KEY_HOME))
                              for (const auto& func : editorReloadShaderEvents) { func(); }
        // Make sure this view matrix is from active camera
        // This should happen after editorEvents
        AM::ViewMat4 = AM::EditorCam.GetViewMatrix();

        // Draw GBuffers --------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, Deferred::GetFBO());
        glDrawBuffers(4, buffers);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        qk::BeginGPUTimer("GBuffers");
        SM::DrawGBuffers();
        std::string timing = qk::EndGPUTimer("GBuffers");
        if (!timing.empty()) std::cout <<  timing;
        /* EDITOR ONLY */ Deferred::DrawMask(); // R8 texture used for outline generation
        // -------------------------------------------

        // Draw deferred shaded to color attachment 0
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        qk::BeginGPUTimer("Shading");
        Deferred::DoShading();
        std::string timing2 = qk::EndGPUTimer("Shading");
        if (!timing2.empty()) std::cout <<  timing2;
        /* EDITOR ONLY */ for (const auto& func : editorDraw3DEvents) { func(); }
        // -------------------------------------------

        // Display -----------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        Deferred::DoPostProcessAndDisplay();
        /* EDITOR ONLY */ if (debugMode == DebugMode::Deferred) Deferred::VisualizeGBuffers();
        /* EDITOR ONLY */ for (const auto& func : editorDrawUIEvents) { func(); }
        /* EDITOR ONLY */ UI::Render();

        Stats::Count(glfwGetTime());
        Stats::DrawStats();
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        // -------------------------------------------

        glfwSwapBuffers(window);
    }

    void Quit()
    {
        glDeleteFramebuffers(1, &Deferred::GetFBO());
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
    
    void errorCallback(int error, const char* description) { fprintf(stderr, "Error: %s\n", description); }
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) { }
}
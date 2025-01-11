#include <iostream>
#include <string>
#include <format>
#include <random>

#include "render_engine.h"
#include "asset_manager.h"
#include "scene_manager.h"
#include "deferred/deffered_manager.h"
#include "editor/object_manipulation.h"
#include "../common/stat_counter.h"
#include "../common/input.h"
#include "../common/qk.h"
#include "../ui/ui.h"
#include "../ui/text_renderer.h"

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
    std::vector<std::function<void()>> editorDraw3DEvent;

    void errorCallback(int error, const char* description);
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    void windowResized(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);

        windowWidth  = width;
        windowHeight = height;
        
        for (const auto& callback : resizeCallbacks) { callback(windowWidth, windowHeight); }
    }

    void RegisterResizeCallback(const std::function<void(int, int)> &callback) { resizeCallbacks.push_back(callback); }
    void RegisterEditorFunction(const std::function<void()>& func) { editorEvents.push_back(func); }
    void RegisterEditorDraw3DFunction(const std::function<void()>& func) { editorDraw3DEvent.push_back(func); }

    void windowMaximized(GLFWwindow* window, int maximized)
    {
        glfwGetWindowSize(window, &windowWidth,& windowHeight);
        windowResized(window, windowWidth, windowHeight);
    }

    void Initialize()
    {
        std::cout << "\n";
        std::cout << "[>] " + std::string(70, '-') + "\n";
        std::cout << "[.] Maeve booting up...\n";
        std::cout << "[>] " + std::string(70, '-') + "\n\n";

        if (glfwInit())
        {
            std::cout << "[:] Successfully initialized GLFW\n";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        
            window = glfwCreateWindow(windowWidth, windowHeight, "Maeve 0.0.1", NULL, NULL);
            glfwMakeContextCurrent(window);
            
            glfwSetErrorCallback(errorCallback);
            glfwSetScrollCallback(window, scrollCallback);
            glfwSetFramebufferSizeCallback(window, windowResized);
            glfwSetWindowMaximizeCallback(window, windowMaximized);

            // glfwSetWindowAttrib(window, GLFW_DECORATED, false);
            // glfwSwapInterval(0);

            // Center Window
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            monitorWidth  = mode->width;
            monitorHeight = mode->height;
            glfwSetWindowPos(window, monitorWidth / 2 - windowWidth / 2, monitorHeight / 2 - windowHeight / 2);

            ToggleFullscreen();
        }
        else std::cout << "[:] Couldn't initialize GLFW\n\n";

        if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) std::cout << "[:] Successfully loaded GLAD\n\n";
        else
        {
            std::cout << "[:] Couldn't load GLAD\n\n";
            glfwTerminate();
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glLineWidth(2);

        Stats::Initialize();
        Input::Initialize();
        Deferred::Initialize();
        SceneManager::Initialize();
        AssetManager::Initialize();
        Manipulation::Initialize();
        Text::Initialize();
        UI::Initialize();

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

        AssetManager::ViewMat4 = AssetManager::EditorCam.GetViewMatrix();
        
        if (Input::KeyDown(GLFW_KEY_SPACE) && Input::KeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);
        if (Input::KeyPressed(GLFW_KEY_F11)) ToggleFullscreen();
        if (Input::KeyPressed(GLFW_KEY_R)) Deferred::S_postprocessQuad->Reload();

        // GBuffers
        glBindFramebuffer(GL_FRAMEBUFFER, Deferred::GetFBO());
        glDrawBuffers(4, buffers);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SceneManager::RenderAll();
        /* EDITOR ONLY */ Deferred::DrawMask();

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        Deferred::DoShading();
        /* EDITOR ONLY */ for (const auto& func : editorDraw3DEvent) { func(); }

        // Display
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        Deferred::DrawPostProcessQuad();
        /* EDITOR ONLY */ Deferred::VisualizeGBuffers();
        /* EDITOR ONLY */ UI::Render();
        /* EDITOR ONLY */ for (const auto& func : editorEvents) { func(); }
        Stats::Count(glfwGetTime());
        Stats::DrawStats();
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
    }

    void Quit()
    {
        glDeleteFramebuffers(1, &Deferred::GetFBO());
        glfwTerminate();
    }

    void ToggleFullscreen()
    {
        if (!isFullscreen)
        {
            glfwMaximizeWindow(window);
            isFullscreen = true;
        }
        else
        {
            glfwRestoreWindow(window);
            isFullscreen = false;
        }
    }

    GLFWwindow* GetWindowPointer() { return window; }
    glm::ivec2  GetWindowSize()    { return glm::ivec2(windowWidth, windowHeight); }
    glm::ivec2  GetMonitorSize()   { return glm::ivec2(monitorWidth, monitorHeight); }
    
    void errorCallback(int error, const char* description) { fprintf(stderr, "Error: %s\n", description); }
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) { }
}
#include <iostream>
#include <string>
#include <format>

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
    std::vector<std::function<void()>> editorFunctions;

    void DrawStats();
    void errorCallback(int error, const char* description);
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    void windowResized(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);

        windowWidth  = width;
        windowHeight = height;
        
        for (const auto& callback : resizeCallbacks)
        {
            callback(width, height);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_BLEND);

        Deferred::DrawFullscreenQuad(Deferred::GetTexture(Deferred::GNormal));
        Deferred::VisualizeGBuffers();
        UI::Render();
        DrawStats();

        glfwSwapBuffers(window);

        // std::cout << "Resized window to " << windowWidth << "x" << windowHeight << "\n\n";
    }

    void RegisterResizeCallback(const std::function<void(int, int)> &callback)
    {
        resizeCallbacks.push_back(callback);
    }

    void RegisterEditorFunction(const std::function<void()>& func)
    {
        editorFunctions.push_back(func);
    }

    void windowMaximized(GLFWwindow* window, int maximized)
    {
        glfwGetWindowSize(window, &windowWidth,& windowHeight);
        windowResized(window, windowWidth, windowHeight);
    }

    void Initialize()
    {
        if (glfwInit())
        {
            std::cout << "Successfully initialized GLFW" << std::endl;
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        
            window = glfwCreateWindow(windowWidth, windowHeight, "Maeve 0.0.1", NULL, NULL);
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
        }
        else std::cout << "Couldn't initialize GLFW" << std::endl;

        if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) std::cout << "Successfully loaded GLAD\n" << std::endl;
        else
        {
            std::cout << "Couldn't load GLAD" << std::endl;
            glfwTerminate();
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glLineWidth(2);

        Statistics::Initialize();
        Input::Initialize();
        Text::Initialize();
        UI::Initialize();
        AssetManager::Initialize();
        Manipulation::Initialize();
        Deferred::CreateFBO(windowWidth, windowHeight);

        windowResized(window, windowWidth, windowHeight);
    }

    void Run()
    {
        while (!glfwWindowShouldClose(window))
        {
            NewFrame();
        }

        Quit();
    }

    int it = 0;
    const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    void NewFrame()
    {
        glfwPollEvents();
        Input::Update();
        for (const auto& func : editorFunctions) { func(); }
        
        if (Input::KeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);
        if (Input::KeyPressed(GLFW_KEY_F11)) ToggleFullscreen();
        if (Input::KeyPressed(GLFW_KEY_P))
        {
            it++;
            SceneManager::Object itobj("name" + std::to_string(it), "loadedmodel");
            itobj.SetPosition(glm::vec3(3 * it, 3, 0));
            SceneManager::AddObject(itobj);
        }

        // GBuffers
        glBindFramebuffer(GL_FRAMEBUFFER, Deferred::GetFBO());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawBuffers(2, buffers);

        //SceneManager::Objects[1].SetRotation(SceneManager::Objects[1].GetRotation() + glm::vec3(0, 100 * Statistics::GetDeltaTime(), 0));
        SceneManager::RenderAll();
        AssetManager::ViewMat4 = AssetManager::EditorCam.GetViewMatrix();

        // Display
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        Deferred::CalculatePBR(AssetManager::EditorCam.Position);
        Deferred::VisualizeGBuffers();
        UI::Render();
        
        Statistics::Count(glfwGetTime());
        DrawStats();
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
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        AssetManager::EditorCam.Speed = glm::clamp(AssetManager::EditorCam.Speed + (float)yoffset, 0.1f, 30.0f);
    }

    float yOffset     = 30;
    float textScaling = 0.35f;
    float timer = 0;
    std::string FPS, ms = "";

    float avgfps = 60.0f;
    float avgms  = 0.016f;
    float alpha = 0.95f;
    void DrawStats()
    {
        std::string memory = std::format("VRAM: {} / {}mb", Statistics::GetVramUsageMb(), Statistics::GetVRAMTotalMb());
        timer += Statistics::GetDeltaTime();
        if (timer >= (1 / (60.0f)))
        {
            timer = 0.0f;
            
            avgfps = alpha * avgfps + (1.0f - alpha) * Statistics::GetFPS();
            avgms  = alpha * avgms + (1.0f - alpha) * Statistics::GetMS();

            FPS = std::format("{:<4} {:>7.2f}", "FPS:", avgfps);
            ms  = std::format("{:<4} {:>7.2f}", "ms:",  avgms);
        }

        std::string meshes = std::format<int>("Meshes in memory: {} ({} triangles)", AssetManager::Meshes.size(), qk::FmtK(AssetManager::UniqueMeshTriCount));
        std::string objects = std::format<int>("Objects in scene: {} ({} triangles)", SceneManager::Objects.size(), qk::FmtK(SceneManager::ObjectsTriCount));
        
        glDisable(GL_DEPTH_TEST);
        float lineSpacing = 20 * Text::GetGlobalTextScaling();
        Text::Render(Statistics::Renderer,                              15, windowHeight - yOffset - 0 * lineSpacing, textScaling);
        Text::Render("Window Size: " + qk::FormatVec(GetWindowSize()),  15, windowHeight - yOffset - 1 * lineSpacing, textScaling);
        Text::Render(FPS,                                               15, windowHeight - yOffset - 3 * lineSpacing, textScaling);
        Text::Render(ms,                                                15, windowHeight - yOffset - 4 * lineSpacing, textScaling);
        Text::Render(memory,                                            15, windowHeight - yOffset - 5 * lineSpacing, textScaling);
        Text::Render(meshes,                                            15, windowHeight - yOffset - 7 * lineSpacing, textScaling);
        Text::Render(objects,                                           15, windowHeight - yOffset - 8 * lineSpacing, textScaling);
        glEnable(GL_DEPTH_TEST);
    }
}
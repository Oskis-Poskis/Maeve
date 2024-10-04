#include <iostream>
#include <string>
#include <format>

#include "../engine/render_engine.h"
#include "../common/stat_counter.h"
#include "../common/input.h"
#include "../ui/text_renderer.h"
#include "../ui/ui.h"
#include "render_engine.h"
#include "asset_manager.h"
#include "scene_manager.h"
#include "deferred/deffered_manager.h"

#include "../common/qk.h"

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

    void SomeStats();
    void errorCallback(int error, const char* description);
    
    void windowResized(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);

        windowWidth  = width;
        windowHeight = height;
        
        for (const auto& callback : resizeCallbacks)
        {
            callback(width, height);
        }
    }

    void RegisterResizeCallback(const std::function<void(int, int)> &callback)
    {
        resizeCallbacks.push_back(callback);
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
            glfwSetFramebufferSizeCallback(window, windowResized);
            glfwSetWindowMaximizeCallback(window, windowMaximized);

            //  glfwSetWindowAttrib(window, GLFW_DECORATED, false);

            // Center Window
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            monitorWidth  = mode->width;
            monitorHeight = mode->height;
            glfwSetWindowPos(window, monitorWidth / 2 - windowWidth / 2, monitorHeight / 2 - windowHeight / 2);
        }
        else
        {
            std::cout << "Couldn't initialize GLFW" << std::endl;
        }

        if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Successfully loaded GLAD\n" << std::endl;
        }
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
        glLineWidth(4);

        Statistics::Initialize();
        Input::Initialize();
        Text::Initialize();
        UI::Initialize();
        AssetManager::Initialize();
        Deferred::CreateFBO(windowWidth, windowHeight);

        windowResized(window, windowWidth, windowHeight);
    }

    const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    void NewFrame(float fov, glm::mat4 vMat, glm::vec3 cPos)
    {
        glfwPollEvents();
        Input::Update();
        
        if (Input::KeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);
        if (Input::KeyPressed(GLFW_KEY_F11)) ToggleFullscreen();

        glBindFramebuffer(GL_FRAMEBUFFER, Deferred::GetFBO());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawBuffers(3, buffers);
        glDisable(GL_BLEND);
        SceneManager::RenderAll(fov, vMat, cPos);
        SceneManager::Objects[1].SetRotation(SceneManager::Objects[1].GetRotation() + glm::vec3(0, 100 * Statistics::GetDeltaTime(), 0));
        glEnable(GL_BLEND);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT);
        Deferred::DrawFullscreenQuad(Deferred::GetTexture(Deferred::GNormal));
        Deferred::VisualizeGBuffers();

        // UI stuff
        UI::Render();
        
        Statistics::Count(glfwGetTime());
        SomeStats();

        glfwSwapBuffers(window);
    }

    void Quit()
    {
        glDeleteFramebuffers(1, &Deferred::GetFBO());
        glfwTerminate();
    }

    void ToggleFullscreen()
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if (!isFullscreen)
        {
            unfullscreenWindowWidth  = windowWidth;
            unfullscreenWindowHeight = windowHeight;
            glfwGetWindowPos(window, &unfullscreenwindowPosX, &unfullscreenwindowPosY);
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            isFullscreen = true;
        }
        else
        {
            glfwSetWindowMonitor(window, nullptr, unfullscreenwindowPosX, unfullscreenwindowPosY, unfullscreenWindowWidth, unfullscreenWindowHeight, mode->refreshRate);
            isFullscreen = false;
        }
    }

    GLFWwindow* GetWindowPointer() { return window; }
    glm::ivec2 GetWindowSize() { return glm::ivec2(windowWidth, windowHeight); }
    glm::ivec2 GetMonitorSize() { return glm::ivec2(monitorWidth, monitorHeight); }
    
    void errorCallback(int error, const char* description) { fprintf(stderr, "Error: %s\n", description); }

    float timer = 0;
    std::string FPS = "";
    std::string ms  = "";
    float yOffset     = 30;
    float textScaling = 0.45f;
    void SomeStats()
    {
        std::string memory = std::format("VRAM: {} / {}mb", Statistics::GetVramUsageMb(), Statistics::GetVRAMTotalMb());
        timer += Statistics::GetDeltaTime();
        if (timer >= (1 / (60.0f)))
        {
            timer = 0.0f;
            FPS = std::format<float>("FPS: {:.2f}", Statistics::GetFPS());
            ms  = std::format<float>("ms:  {:.2f}", Statistics::GetMS());
        }
        std::string meshes = std::format<int>("Meshes in memory: {} ({} triangles)", AssetManager::Meshes.size(), qk::intFmtK(AssetManager::UniqueMeshTriCount));
        std::string objects = std::format<int>("Objects in scene: {} ({} triangles)", SceneManager::Objects.size(), qk::intFmtK(SceneManager::ObjectsTriCount));
        
        glDisable(GL_DEPTH_TEST);
        float lineSpacing = 25 * Text::GetGlobalTextScaling();
        Text::Render(Statistics::Renderer, 15, Engine::GetWindowSize().y - yOffset - 0 * lineSpacing, textScaling);
        if (Statistics::Vendor == "NVIDIA Corporation") Text::Render(memory, 15, Engine::GetWindowSize().y - yOffset - 1 * lineSpacing, textScaling);
        Text::Render(FPS,                  15, Engine::GetWindowSize().y - yOffset - 2 * lineSpacing, textScaling);
        Text::Render(ms,                   15, Engine::GetWindowSize().y - yOffset - 3 * lineSpacing, textScaling);
        Text::Render(meshes,               15, Engine::GetWindowSize().y - yOffset - 5 * lineSpacing, textScaling);
        Text::Render(objects,              15, Engine::GetWindowSize().y - yOffset - 6 * lineSpacing, textScaling);
        glEnable(GL_DEPTH_TEST);
    }
}
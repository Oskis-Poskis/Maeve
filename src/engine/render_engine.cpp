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
    std::vector<std::function<void()>> editorDrawUIEvent;

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
    void RegisterEditorDrawUIFunction(const std::function<void()>& func) { editorDrawUIEvent.push_back(func); }

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
            glfwMakeContextCurrent(window);
            
            glfwSetErrorCallback(errorCallback);
            glfwSetScrollCallback(window, scrollCallback);
            glfwSetFramebufferSizeCallback(window, windowResized);
            glfwSetWindowMaximizeCallback(window, windowMaximized);

            // glfwSetWindowAttrib(window, GLFW_DECORATED, false);
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
        Manipulation::Initialize();
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

    int minDepth = 0;
    int maxDepth = 2;

    const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    void NewFrame()
    {
        glfwPollEvents();
        Input::Update();
        
        if (Input::KeyDown(GLFW_KEY_SPACE) && Input::KeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);
        if (Input::KeyPressed(GLFW_KEY_F11)) ToggleFullscreen();
        if (Input::KeyDown(GLFW_KEY_LEFT_CONTROL) && Input::KeyPressed(GLFW_KEY_Q)) Quit();

        /* EDITOR ONLY */ for (const auto& func : editorEvents) { func(); }

        // Make sure this view matrix is from active camera
        // This should happen after editorEvents
        AM::ViewMat4 = AM::EditorCam.GetViewMatrix();

        // Draw GBuffers --------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, Deferred::GetFBO());
        glDrawBuffers(4, buffers);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SM::RenderAll();
        /* EDITOR ONLY */ Deferred::DrawMask(); // R8 texture used for outline generation
        // -------------------------------------------

        // Draw deferred shaded to color attachment 0
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        Deferred::DoShading();
        /* EDITOR ONLY */ for (const auto& func : editorDraw3DEvent) { func(); }
        // -------------------------------------------

        bool inGame = Input::GetInputContext() == Input::InputContext::Game;
        if (inGame && Input::KeyPressed(GLFW_KEY_DOWN))  maxDepth--;
        if (inGame && Input::KeyPressed(GLFW_KEY_UP))    maxDepth++;
        if (inGame && Input::KeyPressed(GLFW_KEY_LEFT))  minDepth = std::max(minDepth - 1, 0);
        if (inGame && Input::KeyPressed(GLFW_KEY_RIGHT)) minDepth++;

        SM::SceneNode* node = SM::SceneNodes[SM::GetSelectedIndex()];
        /* if (node->GetType() == SM::NodeType::Object_) {
            SM::Object* obj  = SM::GetObjectFromNode(node);
            AM::Mesh&   mesh = AM::Meshes.at(obj->GetMeshID());
            AM::BVH&    bvh  = mesh.bvh;

            // bvh.DrawBVH(bvh.rootIdx, 0, minDepth, maxDepth, obj->GetModelMatrix());

            AM::Ray ray(AM::EditorCam.Position - glm::vec3(0.0f, 0.0f, 0.1f), AM::EditorCam.Front);

            qk::StartTimer();
            bvh.TraverseBVH_Ray(bvh.rootIdx, ray, mesh.vertexData);
            std::cout << "[:] Traverse bvh in " << qk::StopTimer() << " seconds\n";

            qk::DrawLine(ray.origin, ray.origin + ray.direction * 25.0f);
        } */


        // Display -----------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        Deferred::DoPostProcessAndDisplay();
        /* EDITOR ONLY */ Deferred::VisualizeGBuffers();
        /* EDITOR ONLY */ UI::Render();

        if (node->GetType() == SM::NodeType::Object_) {
            SM::Object* obj = SM::GetObjectFromNode(node);
            
            glDisable(GL_DEPTH_TEST);
            glm::vec2 sp   = qk::WorldToScreen(obj->GetPosition());
            int textHeight = Text::CalculateMaxTextAscent("Test", 0.5f);
            int spacing    = 3;
            int numNodes   = AM::Meshes.at(obj->GetMeshID()).bvh.bvhNodes.size();

            Text::RenderCenteredBG("Mesh: " + obj->GetMeshID(), sp.x, sp.y - textHeight, 0.5f, glm::vec3(0.95f));
            Text::RenderCenteredBG(std::format("BVH Nodes: {}", numNodes), sp.x, sp.y - textHeight * 2 - spacing, 0.5f, glm::vec3(0.95f));
            glEnable(GL_DEPTH_TEST);
        }

        /* EDITOR ONLY */ for (const auto& func : editorDrawUIEvent) { func(); }
        Stats::Count(glfwGetTime());
        Stats::DrawStats();
        glDisable(GL_BLEND);
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
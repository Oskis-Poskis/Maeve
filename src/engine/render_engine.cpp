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

#include "../common/qk.h"

namespace Engine
{
    GLFWwindow* _window = NULL;
    int _windowWidth    = 1920;
    int _windowHeight   = 1080;
    int _monitorWidth   = 0;
    int _monitorHeight  = 0;

    bool _fullscreen              = false;
    int _unfullscreenWindowWidth  = 512;
    int _unfullscreenWindowHeight = 512;
    int _unfullscreenwindowPosX   = 0;
    int _unfullscreenwindowPosY   = 0;

    void _windowResized(GLFWwindow* window, int width, int height)
    {
        _windowWidth  = width;
        _windowHeight = height;

        glViewport(0, 0, width, height);

        AssetManager::UpdateOrthoProjMat4(width, height);
    }

    void error_callback(int error, const char* description)
    {
        fprintf(stderr, "Error: %s\n", description);
    }

    void Initialize()
    {
        if (glfwInit())
        {
            std::cout << "Successfully initialized GLFW" << std::endl;
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
            glfwWindowHint(GLFW_SAMPLES, 4);
        
            _window = glfwCreateWindow(_windowWidth, _windowHeight, "Maeve - wip", NULL, NULL);
            glfwMakeContextCurrent(_window);
            glfwSetErrorCallback(error_callback);
            glfwSetFramebufferSizeCallback(_window, _windowResized);

            // glfwSetWindowAttrib(_window, GLFW_DECORATED, false);

            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
             _monitorWidth  = mode->width;
             _monitorHeight = mode->height;
            glfwSetWindowPos(_window, _monitorWidth / 2 - _windowWidth / 2, _monitorHeight / 2 - _windowHeight / 2);
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

        glEnable(GL_MULTISAMPLE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float col = 0.12;
        glClearColor(col, col, col, 1.0f);
        glLineWidth(4);

        Statistics::Initialize();
        Input::Initialize();
        Text::Initialize();
        UI::Initialize();
        AssetManager::Initialize(_windowWidth, _windowHeight);
    }

    float yOffset     = 25;
    float lineSpacing = 30;
    float textScaling = 0.45f;
    void NewFrame(float fov, glm::mat4 vMat, glm::vec3 cPos)
    {
        glfwPollEvents();
        
        Input::Update();
        if (Input::KeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(_window, true);
        if (Input::KeyPressed(GLFW_KEY_F11)) Engine::ToggleFullscreen();
        if (Input::KeyPressed(GLFW_KEY_R)) AssetManager::BPhongShader->Reload();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SceneManager::RenderAll(fov, vMat, cPos);

        std::string memory = std::format("VRAM: {} / {}mb", Statistics::GetVramUsageMb(), Statistics::GetVRAMTotalMb());
        std::string FPS = std::format<float>("FPS: {:.2f}", Statistics::GetFPS());
        std::string ms  = std::format<float>("ms:  {:.2f}", Statistics::GetMS());
        std::string meshes = std::format<int>("Meshes in memory: {} ({} triangles)", AssetManager::Meshes.size(), qk::intFmtK(AssetManager::UniqueMeshTriCount));
        std::string objects = std::format<int>("Objects in scene: {} ({} triangles)", SceneManager::Objects.size(), qk::intFmtK(SceneManager::ObjectsTriCount));
        
        glDisable(GL_DEPTH_TEST);
        Text::Render(Statistics::Renderer, 15, Engine::GetWindowSize().y - yOffset - 0 * lineSpacing, textScaling);
        if (Statistics::Vendor == "NVIDIA Corporation") Text::Render(memory, 15, Engine::GetWindowSize().y - yOffset - 1 * lineSpacing, textScaling);
        Text::Render(FPS,                  15, Engine::GetWindowSize().y - yOffset - 2 * lineSpacing, textScaling);
        Text::Render(ms,                   15, Engine::GetWindowSize().y - yOffset - 3 * lineSpacing, textScaling);
        Text::Render(meshes,               15, Engine::GetWindowSize().y - yOffset - 5 * lineSpacing, textScaling);
        Text::Render(objects,              15, Engine::GetWindowSize().y - yOffset - 6 * lineSpacing, textScaling);
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        UI::Render();
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnable(GL_DEPTH_TEST);

        Statistics::Count(glfwGetTime());
        
        glfwSwapBuffers(_window);
    }

    void Quit()
    {
        glfwTerminate();
    }

    GLFWwindow* GetWindowPointer()
    {
        return _window;
    }

    glm::ivec2 GetWindowSize()
    {
        return glm::ivec2(_windowWidth, _windowHeight);
    }

    glm::ivec2 GetMonitorSize()
    {
        return glm::ivec2(_monitorWidth, _monitorHeight);
    }

    void ToggleFullscreen()
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if (!_fullscreen)
        {
            _unfullscreenWindowWidth  = _windowWidth;
            _unfullscreenWindowHeight = _windowHeight;
            glfwGetWindowPos(_window, &_unfullscreenwindowPosX, &_unfullscreenwindowPosY);
            glfwSetWindowMonitor(_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            _fullscreen = true;
        }
        else
        {
            glfwSetWindowMonitor(_window, nullptr, _unfullscreenwindowPosX, _unfullscreenwindowPosY, _unfullscreenWindowWidth, _unfullscreenWindowHeight, mode->refreshRate);
            _fullscreen = false;
        }
    }
}
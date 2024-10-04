#include <iostream>
#include <string>
#include <random>

#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"

#include "engine/render_engine.h"
#include "engine/scene_manager.h"
#include "engine/asset_manager.h"
#include "common/camera.h"
#include "common/shader.h"

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), 70, -90.0f, 0.0f);

int main()
{
    Engine::Initialize();

    glfwSetScrollCallback(Engine::GetWindowPointer(), scroll_callback);
    
    SceneManager::Object plane("Plane", "plane");
    plane.SetScale(glm::vec3(25.0f));
    SceneManager::AddObject(plane);

    AssetManager::ObjLoader::Load("/../res/objs/guy.obj", "loadedmodel");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib360(0, 360);

    int num = 15;
    float spacing = 3.1f;
    for (int x = 0; x < num; x++)
    {
        for (int z = 0; z < num; z++)
        {
            std::string name = "obj_" + std::to_string(x) + "-" + std::to_string(z);
            SceneManager::Object itobj(name, "loadedmodel");
            itobj.SetPosition(glm::vec3(x * spacing - ((num - 1) * spacing) / 2.0f, 0, z * spacing - ((num - 1) * spacing) / 2.0f));
            //itobj.SetRotation(glm::vec3(distrib360(gen), distrib360(gen), distrib360(gen)));
            itobj.SetScale(glm::vec3(0.25f));
            SceneManager::AddObject(itobj);
        }   
    }

    while (!glfwWindowShouldClose(Engine::GetWindowPointer()))
    {
        camera.Update();

        Engine::NewFrame(camera.Fov, camera.GetViewMatrix(), camera.Position);
    }

    Engine::Quit();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.Speed = glm::clamp(camera.Speed + (float)yoffset, 0.1f, 30.0f);
}
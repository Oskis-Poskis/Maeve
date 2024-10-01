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

char mapToChar(int coord, int maxVal) {
    // Normalize the value to the range 0-25 (mapping to A-Z)
    int normalized = (coord + maxVal) % 26; // Ensure value stays within the alphabet
    return 'A' + normalized;
}

std::string generateName(int x, int y, int z, int num) {

    // Mapping x, y, z to characters
    char charX = mapToChar(x, num);
    char charY = mapToChar(y, num);
    char charZ = mapToChar(z, num);
    
    // Combine them into a string for a name
    std::string name;
    name.push_back(charX);
    name.push_back(charY);
    name.push_back(charZ);
    
    return name;
}

int main()
{
    Engine::Initialize();

    glfwSetScrollCallback(Engine::GetWindowPointer(), scroll_callback);
    
    SceneManager::Object plane("Plane", "plane");
    plane.SetScale(glm::vec3(20.0f));
    SceneManager::AddObject(plane);

    AssetManager::ObjLoader::Load("/../res/objs/teapot.obj", "loadedmodel");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib360(0, 360);

    int num = 2;
    float spacing = 2.0f;
    for (int x = -num; x <= num; x++)
    {
        for (int y = 0; y < num * 2 + 1; y++)
        {
            for (int z = -num; z <= num; z++)
            {
                std::string name = "Teapot-" + generateName(x, y, z, 2);
                SceneManager::Object itobj(name, "loadedmodel");
                itobj.SetPosition(glm::vec3(x * spacing, y * spacing + spacing, z * spacing));
                itobj.SetRotation(glm::vec3(distrib360(gen), distrib360(gen), distrib360(gen)));
                itobj.SetScale(glm::vec3(0.3f));
                SceneManager::AddObject(itobj);
            }
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
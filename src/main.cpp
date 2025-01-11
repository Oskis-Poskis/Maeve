#include "engine/render_engine.h"
#include "engine/asset_manager.h"
#include "engine/scene_manager.h"
#include "common/qk.h"

int main()
{
    Engine::Initialize();
 
    AssetManager::IO::LoadObj("res/objs/armadillo.obj",      "loadedmodel0");
    AssetManager::IO::LoadObj("res/objs/cube.obj",           "loadedmodel1");
    AssetManager::IO::LoadObj("res/objs/teapot.obj",         "loadedmodel2");
    AssetManager::IO::LoadObj("res/objs/sphere.obj",         "loadedmodel3");
    AssetManager::IO::LoadObj("res/objs/teapot.obj",         "loadedmodel4");
    AssetManager::IO::LoadObj("res/objs/suzanne.obj",        "loadedmodel5");
    AssetManager::IO::LoadObj("res/objs/suzanne_smooth.obj", "loadedmodel6");

    SceneManager::Light light1("light1", SceneManager::LightType::Point, glm::vec3(0, 3, 0),  10.0f, glm::vec3(1, 1, 1), 10.0f);
    SceneManager::Light light2("light2", SceneManager::LightType::Point, glm::vec3(-7, 3, 0), 10.0f, glm::vec3(0, 1, 1),  2.5f);
    SceneManager::Light light3("light3", SceneManager::LightType::Point, glm::vec3(7, 3, 0),  10.0f, glm::vec3(1, 0, 0),  5.0f);

    SceneManager::AddLight(light1);
    SceneManager::AddLight(light2);
    SceneManager::AddLight(light3);

    int num = 5;
    int spacing = 4;
    for (int i = 0; i < num * num * num; i++)
    {
        int xdir = i % num;
        int ydir = i /(num * num);
        int zdir = (i / num) % num;

        std::string mesh = "loadedmodel6"; // std::to_string(qk::RandomInt(0, 6))
        SceneManager::Object myobj("monkey_" + std::to_string(i), mesh);
        myobj.SetPosition(glm::vec3(xdir * spacing - ((num - 1) * spacing) / 2.0f, ydir * spacing, zdir * spacing - ((num - 1) * spacing) / 2.0f));
        SceneManager::AddObject(myobj);
    }
 
    Engine::Run();
}
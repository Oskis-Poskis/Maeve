#include "engine/render_engine.h"
#include "engine/scene_manager.h"
#include "engine/asset_manager.h"

int main()
{
    Engine::Initialize();
    
    AssetManager::IO::LoadObj("res/objs/armadillo.obj",      "loadedmodel0");
    // AssetManager::IO::LoadObj("res/objs/cube.obj",           "loadedmodel1");
    // AssetManager::IO::LoadObj("res/objs/dragon.obj",         "loadedmodel2");
    // AssetManager::IO::LoadObj("res/objs/sphere.obj",         "loadedmodel3");
    // AssetManager::IO::LoadObj("res/objs/suzanne_smooth.obj", "loadedmodel4");
    // AssetManager::IO::LoadObj("res/objs/suzanne.obj",        "loadedmodel5");
    // AssetManager::IO::LoadObj("res/objs/teapot.obj",         "loadedmodel6");
    // AssetManager::IO::LoadObj("res/objs/xyzrgb_dragon.obj",  "loadedmodel7");
    // AssetManager::IO::LoadObj("res/objs/armadillo_high.obj", "loadedmodel8");
    // AssetManager::IO::LoadObj("../res/objs/wrona.ob",        "failedmodel");
    
    SceneManager::Object plane("Plane", "plane");
    plane.SetScale(glm::vec3(10.0f));
    SceneManager::AddObject(plane);

    Engine::Run();
}
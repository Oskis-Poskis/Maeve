#include "engine/render_engine.h"
#include "engine/scene_manager.h"
#include "engine/asset_manager.h"

int main()
{
    Engine::Initialize();
    
    AssetManager::IO::LoadObj("../res/objs/dragon.obj", "loadedmodel");
    AssetManager::IO::LoadObj("../res/objs/armadillo.obj", "loadedmodel1");
    AssetManager::IO::LoadObj("../res/objs/xyzrgb_dragon.obj", "loadedmodel2");
    AssetManager::IO::LoadObj("../res/objs/armadillo_high.obj", "loadedmodel4");
    AssetManager::IO::LoadObj("../res/objs/wrona.ob", "loadedmodel4");
    
    SceneManager::Object plane("Plane", "plane");
    plane.SetScale(glm::vec3(10.0f));
    SceneManager::AddObject(plane);
    
    SceneManager::Object itobj("pepo", "loadedmodel");
    itobj.SetPosition(glm::vec3(0, 3, 0));
    SceneManager::AddObject(itobj);

    Engine::Run();
}
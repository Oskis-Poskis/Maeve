#include "engine/render_engine.h"
#include "engine/asset_manager.h"
#include "engine/scene_manager.h"
#include "common/command_parser.h"
#include "common/stat_counter.h"

#include <ctime>
#include <iostream>

int main()
{
    Engine::Initialize();
    
    AM::IO::LoadObjAsync("res/objs/suzanne_smooth.obj", "loaded_1");
    AM::IO::LoadObjAsync("res/objs/cube.obj",           "loaded_2");
    
    SM::Object* suzanne = new SM::Object("Suzanne", "loaded_1");
    suzanne->SetPosition({0, 0, 3});
    
    SM::Object* floor  = new SM::Object("Floor", "loaded_2");
    floor->SetScale({7.5f, 7.5f, 0.1f});
    
    SM::Object* cube  = new SM::Object("Cube", "loaded_2");
    cube->SetPosition({2.0f, 0.0f, 0.7f});
    
    SM::AddNode(suzanne);
    SM::AddNode(floor);
    SM::AddNode(cube);

    // add -o "MyObject" -m "Mesh1"
    // add -o "MyObject" -m "Mesh1" -p vec3(0.0, 0.0, 0.0)

    // PC::RunTest("add -o \"MyObject\" -m \"Mesh1\" -p vec3(1.0, 2.0, 3.0)");

    Engine::Run();
}
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

    std::vector<std::string> meshIDs = {
        "loaded_1", "loaded_2", "loaded_3", "loaded_4", "loaded_5", "loaded_6", "loaded_7",
        "loaded_8", "loaded_9", "loaded_10", "loaded_11", "loaded_12", "loaded_13", "loaded_14"
    };

    AM::IO::LoadObjAsync("res/objs/suzanne_smooth.obj", meshIDs[0]);
    AM::IO::LoadObjAsync("res/objs/suzanne.obj",        meshIDs[1]);
    AM::IO::LoadObjAsync("res/objs/sphere.obj",         meshIDs[2]);
    AM::IO::LoadObjAsync("res/objs/cube.obj",           meshIDs[3]);
    AM::IO::LoadObjAsync("res/objs/teapot.obj",         meshIDs[4]);
    AM::IO::LoadObjAsync("res/objs/sphere.obj",         meshIDs[5]);
    AM::IO::LoadObjAsync("res/objs/cube.obj",           meshIDs[6]);
    AM::IO::LoadObjAsync("res/objs/teapot.obj",         meshIDs[7]);
    AM::IO::LoadObjAsync("res/objs/suzanne.obj",        meshIDs[8]);
    AM::IO::LoadObjAsync("res/objs/sphere.obj",         meshIDs[9]);
    AM::IO::LoadObjAsync("res/objs/cube.obj",           meshIDs[10]);
    AM::IO::LoadObjAsync("res/objs/teapot.obj",         meshIDs[11]);
    AM::IO::LoadObjAsync("res/objs/sphere.obj",         meshIDs[12]);
    AM::IO::LoadObjAsync("res/objs/cube.obj",           meshIDs[13]);

    // SM::Object* suzanne = new SM::Object("Suzanne", "loaded_1");
    // suzanne->SetPosition({0, 0, 3});
    
    // SM::Object* floor  = new SM::Object("Floor", "loaded_4");
    // floor->SetScale({7.5f, 7.5f, 0.1f});
    SM::Object* floor  = new SM::Object("Floor", "loaded_4");
    floor->SetScale({35.0f, 35.0f, 0.1f});
    floor->SetPosition({0.0f, 15.0f, -20.0f});
    
    // SM::Object* cube  = new SM::Object("Cube", "loaded_4");
    // cube->SetPosition({2.0f, 0.0f, 0.7f});
    
    // SM::AddNode(suzanne);
    // SM::AddNode(floor);
    // SM::AddNode(cube);
    SM::AddNode(floor);

    int num = 7;
    int spacing = 4;

    std::srand(static_cast<unsigned>(69));
    for (int i = 0; i < num * num * num; i++)
    {
        int xdir = i % num;
        int ydir = i / (num * num);
        int zdir = (i / num) % num;

        std::string mesh = meshIDs[rand() % meshIDs.size()];

        SM::Object* myobj = new SM::Object("obj_" + std::to_string(i), mesh);
        myobj->SetPosition(glm::vec3(
            xdir * spacing - ((num - 1) * spacing) / 2.0f,
            ydir * spacing,
            zdir * spacing - ((num - 1) * spacing) / 2.0f
        ));

        // Random rotation in degrees (0–360)
        float rx = static_cast<float>(rand() % 360);
        float ry = static_cast<float>(rand() % 360);
        float rz = static_cast<float>(rand() % 360);
        myobj->Rotate(glm::vec3(rx, ry, rz));

        SM::AddNode(myobj);
    }

    // int numLights = 3;
    // int spacingLight = 8;

    // for (int i = 0; i < numLights * numLights * numLights; i++)
    // {
    //     int x = i % numLights;
    //     int y = i / (numLights * numLights);
    //     int z = (i / numLights) % numLights;

    //     glm::vec3 pos = glm::vec3(
    //         x * spacingLight - ((numLights - 1) * spacingLight) / 2.0f,
    //         y * spacingLight,
    //         z * spacingLight - ((numLights - 1) * spacingLight) / 2.0f
    //     );

    //     glm::vec3 color = glm::vec3(
    //         static_cast<float>(rand() % 100) / 100.0f,
    //         static_cast<float>(rand() % 100) / 100.0f,
    //         static_cast<float>(rand() % 100) / 100.0f
    //     );

    //     std::string name = "auto_light_" + std::to_string(i);
    //     SM::Light* light = new SM::Light(name, SM::LightType::Point, pos, 10.0f, color, 10.0f);
    //     SM::AddNode(light);
    // }

    // add -o "MyObject" -m "Mesh1"
    // add -o "MyObject" -m "Mesh1" -p vec3(0.0, 0.0, 0.0)

    // PC::RunTest("add -o \"MyObject\" -m \"Mesh1\" -p vec3(1.0, 2.0, 3.0)");

    Engine::Run();
}
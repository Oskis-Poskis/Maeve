#include "engine/render_engine.h"
#include "engine/asset_manager.h"
#include "engine/scene_manager.h"
#include "common/command_parser.h"

#include <ctime>

int main()
{
    Engine::Initialize();
 
    // AM::IO::LoadObj("res/objs/suzanne_smooth.obj", "loaded_1");
    // AM::IO::LoadObj("res/objs/suzanne.obj",        "loaded_2");
    // AM::IO::LoadObj("res/objs/sphere.obj",         "loaded_3");
    // AM::IO::LoadObj("res/objs/cube.obj",           "loaded_4");
    // AM::IO::LoadObj("res/objs/teapot.obj",         "loaded_5");
    AM::IO::LoadObj("res/objs/xyzrgb_dragon.obj",  "Dragon");
    SM::Object* dragon = new SM::Object("Dragon", "Dragon");
    SM::Object* plane  = new SM::Object("Plane", "plane");
    plane->Rotate({90, 0, 0});
    plane->SetScale({5, 5, 5});
    SM::AddNode(dragon);
    SM::AddNode(plane);

    // std::vector<std::string> meshIDs = { "loaded_1", "loaded_2", "loaded_3", "loaded_4", "loaded_5" };

    // int num = 8;
    // int spacing = 4;

    // std::srand(static_cast<unsigned>(std::time(nullptr)));
    // for (int i = 0; i < num * num * num; i++)
    // {
    //     int xdir = i % num;
    //     int ydir = i / (num * num);
    //     int zdir = (i / num) % num;

    //     std::string mesh = meshIDs[rand() % meshIDs.size()];

    //     SM::Object* myobj = new SM::Object("obj_" + std::to_string(i), mesh);
    //     myobj->SetPosition(glm::vec3(
    //         xdir * spacing - ((num - 1) * spacing) / 2.0f,
    //         ydir * spacing,
    //         zdir * spacing - ((num - 1) * spacing) / 2.0f
    //     ));

    //     // Random rotation in degrees (0–360)
    //     float rx = static_cast<float>(rand() % 360);
    //     float ry = static_cast<float>(rand() % 360);
    //     float rz = static_cast<float>(rand() % 360);
    //     myobj->Rotate(glm::vec3(rx, ry, rz));

    //     SM::AddNode(myobj);
    // }

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

    PC::RunTest("add -o \"MyObject\" -m \"Mesh1\" -p vec3(1.0, 2.0, 3.0)");

    Engine::Run();
}
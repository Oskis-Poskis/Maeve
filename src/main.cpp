#include "engine/render_engine.h"
#include "engine/asset_manager.h"
#include "engine/scene_manager.h"

#include <ctime>

int main()
{
    Engine::Initialize();
 
    AM::IO::LoadObj("res/objs/suzanne_smooth.obj", "loaded_1");
    AM::IO::LoadObj("res/objs/suzanne.obj",        "loaded_2");
    AM::IO::LoadObj("res/objs/sphere.obj",         "loaded_3");
    AM::IO::LoadObj("res/objs/cube.obj",           "loaded_4");
    AM::IO::LoadObj("res/objs/teapot.obj",         "loaded_5");

    std::vector<std::string> meshIDs = { "loaded_1", "loaded_2", "loaded_3", "loaded_4", "loaded_5" };

    int num = 10;
    int spacing = 4;

    std::srand(static_cast<unsigned>(std::time(nullptr)));
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

    SM::Light light1("Point Light1", SM::LightType::Point, glm::vec3( 0, 0, 2.5f), 10.0f, glm::vec3(1, 1, 1), 5.0f);
    SM::Light light2("Point Light2", SM::LightType::Point, glm::vec3(-7, 0, 2.5f), 10.0f, glm::vec3(0, 1, 1), 5.0f);
    SM::Light light3("Point Light3", SM::LightType::Point, glm::vec3( 7, 0, 2.5f), 10.0f, glm::vec3(1, 0, 0), 5.0f);
    // SM::AddNode(&light2);
    // SM::AddNode(&light3);
    // SM::AddNode(&light1);
 
    Engine::Run();
}
#include "engine/render_engine.h"
#include "engine/asset_manager.h"
#include "engine/scene_manager.h"

int main()
{
    Engine::Initialize();
 
    AM::IO::LoadObj("res/objs/suzanne_smooth.obj", "loaded1");
    SM::Object obj("Loaded Model", "loaded1");
    SM::AddNode(&obj);

    /* int num = 5;
    int spacing = 4;
    for (int i = 0; i < num * num * num; i++)
    {
        int xdir = i % num;
        int ydir = i /(num * num);
        int zdir = (i / num) % num;

        std::string mesh = "suzanne";
        SM::Object* myobj = new SM::Object("monkey_" + std::to_string(i), mesh);
        myobj->SetPosition(glm::vec3(xdir * spacing - ((num - 1) * spacing) / 2.0f, ydir * spacing, zdir * spacing - ((num - 1) * spacing) / 2.0f));
        SM::AddNode(myobj);
    } */

    SM::Light light1("Point Light1", SM::LightType::Point, glm::vec3( 0, 0, 4.0f), 10.0f, glm::vec3(1, 1, 1), 5.0f);
    SM::Light light2("Point Light2", SM::LightType::Point, glm::vec3(-7, 0, 2.5f), 10.0f, glm::vec3(0, 1, 1), 5.0f);
    SM::Light light3("Point Light3", SM::LightType::Point, glm::vec3( 7, 0, 2.5f), 10.0f, glm::vec3(1, 0, 0), 5.0f);
    SM::AddNode(&light1);
    SM::AddNode(&light2);
    SM::AddNode(&light3);
 
    Engine::Run();
}
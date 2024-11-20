#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "object_manipulation.h"
#include "../render_engine.h"
#include "../scene_manager.h"
#include "../asset_manager.h"
#include "../../common/input.h"
#include "../../common/stat_counter.h"

namespace Manipulation
{
    void Transform()
    {
        glm::vec4 ss = glm::vec4((Input::GetMouseX() / Engine::GetWindowSize().x) * 2.0f - 1.0f, (1.0f - (Input::GetMouseY() / Engine::GetWindowSize().y)) * 2.0f - 1.0f, 1.0f, 1.0f);
        glm::vec3 dir = glm::normalize(glm::vec3(glm::inverse(AssetManager::ProjMat4 * AssetManager::ViewMat4) * ss));

        //glm::vec3 orig = SceneManager::Objects[11].GetRotation();
        //SceneManager::Objects[11].SetRotation(glm::vec3(Input::GetMouseY() / 10.0f, orig.y, orig.z));

        //std::cout << SceneManager::Objects[11].GetRotation().x << " " << SceneManager::Objects[11].GetRotation().y << " " << SceneManager::Objects[11].GetRotation().z << "\n";
    }

    void Initialize()
    {
        Engine::RegisterEditorFunction(Transform);
    }
}
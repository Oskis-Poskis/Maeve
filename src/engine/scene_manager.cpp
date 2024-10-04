#include "../../lib/glad/glad.h"
#include "../../lib/glm/gtc/matrix_transform.hpp"
#include "../../lib/glm/gtc/type_ptr.hpp"

#include "scene_manager.h"
#include "render_engine.h"
#include "asset_manager.h"

namespace SceneManager
{

    void AddObject(Object Object)
    {
        Objects.push_back(Object);
        CalculateObjectsTriCount();
    }

    void RenderAll(float fov, glm::mat4 vMat, glm::vec3 cPos)
    {
        AssetManager::S_GBuffers->Use();
        glm::mat4 proj = glm::perspective(glm::radians(fov), (float)Engine::GetWindowSize().x / Engine::GetWindowSize().y, 0.1f, 1000.0f);
        AssetManager::S_GBuffers->SetMatrix4("projection", proj);
        AssetManager::S_GBuffers->SetMatrix4("view", vMat);
        // AssetManager::S_GBuffers->SetVector3("viewPos", cPos);

        for (auto it = AssetManager::Meshes.begin(); it != AssetManager::Meshes.end(); it++)
        {
            int numElements = it->second.TriangleCount * 3;
            glBindVertexArray(it->second.VAO);

            for (int j = 0; j < SceneManager::Objects.size(); j++)
            {
                if (SceneManager::Objects[j].MeshID == it->first)
                {
                    AssetManager::S_GBuffers->SetMatrix4("model", SceneManager::Objects[j].GetModelMatrix());
                    if (it->second.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
                    else glDrawArrays(GL_TRIANGLES, 0, it->second.TriangleCount * 3);
                }
            }
        }
    }

    Object::Object(std::string Name, std::string MeshID)
    {
        _name        = Name;
        _position    = glm::vec3(0.0f);
        _rotation    = glm::vec3(0.0f);
        _scale       = glm::vec3(1.0f);
        _modelMatrix = glm::mat4(1.0);
        this->MeshID = MeshID;

        SetModelMatrix();
    }

    void Object::SetPosition(glm::vec3 Position)
    {
        _position = Position;
        _modelMatrix = glm::translate(_modelMatrix, _position);
    }

    void Object::SetRotation(glm::vec3 Rotation)
    {
        glm::vec3 rotOffset = Rotation - _rotation;
           
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(rotOffset.x), glm::vec3(1.0f, 0.0f, 0.0f));
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(rotOffset.y), glm::vec3(0.0f, 1.0f, 0.0f));
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(rotOffset.z), glm::vec3(0.0f, 0.0f, 1.0f));

        _rotation = Rotation;
    }

    void Object::SetScale(glm::vec3 Scale)
    {
        _scale = Scale;
        _modelMatrix = glm::scale(_modelMatrix, _scale);
    }

    void Object::SetModelMatrix()
    {
        _modelMatrix = glm::translate(_modelMatrix, _position);
        
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        
        _modelMatrix = glm::scale(_modelMatrix, _scale);
    }

    void Object::SetName(std::string Name)
    {
        _name = Name;
    }

    glm::vec3 Object::GetPosition()
    {
        return _position;
    }

    glm::vec3 Object::GetRotation()
    {
        return _rotation;
    }

    glm::vec3 Object::GetScale()
    {
        return _scale;
    }
    
    std::string Object::GetName()
    {
        return _name;
    }

    glm::mat4 Object::GetModelMatrix()
    {
        return _modelMatrix;
    }

    void CalculateObjectsTriCount()
    {
        int numtris = 0;
        for (int i = 0; i < SceneManager::Objects.size(); i++)
        {
            numtris += AssetManager::Meshes.at(SceneManager::Objects[i].MeshID).TriangleCount;
        }

        ObjectsTriCount = numtris;   
    }
}
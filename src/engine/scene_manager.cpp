#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene_manager.h"
#include "render_engine.h"
#include "asset_manager.h"
#include "../common/camera.h"

namespace SceneManager
{
    void AddObject(Object Object)
    {
        Objects.push_back(Object);
        ObjectNames.push_back(Object.GetName());
        CalculateObjectsTriCount();
    }

    void RenderAll()
    {
        AssetManager::S_GBuffers->Use();
        AssetManager::S_GBuffers->SetMatrix4("projection", AssetManager::ProjMat4);
        AssetManager::S_GBuffers->SetMatrix4("view", AssetManager::ViewMat4);

        for (auto const& mesh : AssetManager::Meshes)
        {
            int numElements = mesh.second.TriangleCount * 3;
            glBindVertexArray(mesh.second.VAO);

            for (auto &obj : SceneManager::Objects)
            {
                if (obj.GetMeshID() == mesh.first)
                {
                    AssetManager::S_GBuffers->SetMatrix4("model", obj.GetModelMatrix());
                    if (mesh.second.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
                    else glDrawArrays(GL_TRIANGLES, 0, mesh.second.TriangleCount * 3);
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
        _meshID = MeshID;

        SetModelMatrix(glm::vec3(0), glm::vec3(0), glm::vec3(1));
    }

    void Object::SetPosition(glm::vec3 Position)
    {
        glm::vec3 posOffset = Position - _position;

        _position = Position;

        SetModelMatrix(posOffset, glm::vec3(0), glm::vec3(1));
    }

    void Object::SetRotation(glm::vec3 Rotation)
    {
        glm::vec3 rotOffset = Rotation - _rotation;
        _rotation = Rotation;

        SetModelMatrix(glm::vec3(0), rotOffset, glm::vec3(1));
    }

    void Object::SetScale(glm::vec3 Scale)
    {
        glm::vec3 scaleOffset = Scale - _scale;
        _scale = Scale;

        SetModelMatrix(glm::vec3(0), glm::vec3(0), scaleOffset);
    }

    void Object::SetModelMatrix(glm::vec3 PosOffset, glm::vec3 RotOffset, glm::vec3 ScaleOffset)
    {
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(RotOffset.x), glm::vec3(1.0f, 0.0f, 0.0f));
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(RotOffset.y), glm::vec3(0.0f, 1.0f, 0.0f));
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(RotOffset.z), glm::vec3(0.0f, 0.0f, 1.0f));

        _modelMatrix = glm::translate(_modelMatrix, PosOffset);
        
        _modelMatrix = glm::scale(_modelMatrix, ScaleOffset);
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

    std::string Object::GetMeshID()
    {
        return _meshID;
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
            numtris += AssetManager::Meshes.at(SceneManager::Objects[i].GetMeshID()).TriangleCount;
        }

        ObjectsTriCount = numtris;   
    }

    std::vector<std::string> ObjetNames()
    {
        std::vector<std::string> names;
        for (Object &obj : Objects)
        {
            std::string name = obj.GetName();
            names.push_back(name);
        }
        return names;
    }
}
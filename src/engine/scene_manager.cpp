#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene_manager.h"
#include "render_engine.h"
#include "asset_manager.h"
#include "../common/camera.h"
#include "../common/qk.h"

#include <iostream>

namespace SceneManager
{
    int _selectedObject;

    void Initialize()
    {
        Engine::RegisterEditorDraw3DFunction(DrawLights);
    }

    void AddObject(Object Object)
    {
        Objects.push_back(Object);
        ObjectNames.push_back(Object.GetName());
        CalculateObjectsTriCount();
    }

    void AddLight(Light Light)
    {
        Lights.push_back(Light);
    }

    void SelectObject(int Index)
    {
        _selectedObject = glm::min(Index, (int)Objects.size() - 1);
    }

    int GetSelectedIndex()
    {
        return _selectedObject;
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

            // todo -> unordered map with key as material names, value is list of objects with that material
            int index = 0;
            for (auto &obj : SceneManager::Objects)
            {
                if (obj.GetMeshID() == mesh.first)
                {
                    AssetManager::S_GBuffers->SetMatrix4("model", obj.GetModelMatrix());
                    if (mesh.second.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
                    else glDrawArrays(GL_TRIANGLES, 0, mesh.second.TriangleCount * 3);
                }
                index++;
            }
        }
    }

    void DrawLights()
    {
        for (auto &light : SceneManager::Lights)
        {
            qk::DrawScreenAlignedPlane(light.GetPosition(), glm::vec3(0.15f), light.GetColor());
        }
    }

    Object::Object(std::string Name, std::string MeshID)
    {
        _name        = Name;
        _position    = glm::vec3(0.0f);
        _rotation    = glm::vec3(0.0f);
        _scale       = glm::vec3(1.0f);
        _modelMatrix = glm::mat4(1.0f);
        _meshID      = MeshID;

        RecalculateMat4();
    }

    void Object::SetPosition(glm::vec3 Position)
    {
        _position = Position;
        RecalculateMat4();
    }

    void Object::SetRotation(glm::vec3 Rotation)
    {
        _rotation = Rotation;
        RecalculateMat4();
    }

    void Object::SetScale(glm::vec3 Scale)
    {
        _scale = Scale;
        RecalculateMat4();
    }

    void Object::RecalculateMat4()
    {
        glm::mat4 identity(1.0f);

        _modelMatrix = glm::scale(identity, _scale);

        _modelMatrix = glm::translate(_modelMatrix, _position);

        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
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

    glm::mat4 &Object::GetModelMatrix()
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

    Light::Light(std::string Name, LightType Type)
    {
        _name  = Name;
        _type  = Type;

        _position  = glm::vec3(0.0f);
        _radius    = 10.0f;
        _color     = glm::vec3(1.0, 0.0, 0.0);
        _intensity = 5.0f;
    }

    Light::Light(std::string Name, LightType Type, glm::vec3 Position, float Radius, glm::vec3 Color, float Intensity)
    {
        _name  = Name;
        _type  = Type;

        _position  = Position;
        _radius    = Radius;
        _color     = Color;
        _intensity = Intensity;
    }

    void Light::SetPosition(glm::vec3 Position)
    {
        _position = Position;
    }

    void Light::SetColor(glm::vec3 Color)
    {
        _color = Color;
    }

    void Light::SetIntensity(float Intensity)
    {
        _intensity = Intensity;
    }

    void Light::SetRadius(float Radius)
    {
        _radius = Radius;
    }

    glm::vec3 Light::GetPosition()
    {
        return _position;
    }
    
    glm::vec3 Light::GetColor()
    {
        return _color;
    }

    float Light::GetIntensity()
    {
        return _intensity;
    }
}
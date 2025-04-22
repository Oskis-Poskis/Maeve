#include <iostream>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene_manager.h"
#include "render_engine.h"
#include "asset_manager.h"
#include "../common/camera.h"
#include "../common/qk.h"

namespace SM
{
    int _selectedObject;

    void DrawLights();
    void DrawOrigin();

    void Initialize()
    {
        Engine::RegisterEditorDraw3DFunction(DrawLights);
        Engine::RegisterEditorDraw3DFunction(DrawOrigin);
    }

    void AddNode(Object* Object)
    {
        SceneNodes.push_back(Object);
        SceneNodeNames.push_back(Object->GetName());
        CalculateObjectsTriCount();

        NumObjects++;
        printf("Added object \"%s\" with mesh \"%s\"\n", Object->GetName().c_str(), Object->GetMeshID().c_str());
    }

    void AddNode(Light* Light)
    {
        SceneNodes.push_back(Light);
        SceneNodeNames.push_back(Light->GetName());

        NumLights++;
        printf("Added light \"%s\"\n", Light->GetName().c_str());
    }

    void SelectSceneType(int Index)
    {
        _selectedObject = glm::min(Index, (int)SceneNodes.size() - 1);
    }

    int GetSelectedIndex()
    {
        return _selectedObject;
    }

    void RenderAll()
    {
        AM::S_GBuffers->Use();
        AM::S_GBuffers->SetMatrix4("projection", AM::ProjMat4);
        AM::S_GBuffers->SetMatrix4("view", AM::ViewMat4);

        for (auto const& mesh : AM::Meshes)
        {
            int numElements = mesh.second.TriangleCount * 3;
            glBindVertexArray(mesh.second.VAO);

            // todo -> unordered map with key as material names, value is list of objects with that material
            int index = 0;
            for (auto &node : SM::SceneNodes)
            {
                if (node->GetType() == NodeType::Object_)
                {
                    Object* object = dynamic_cast<Object*>(node);
                    if (object->GetMeshID() == mesh.first)
                    {
                        AM::S_GBuffers->SetMatrix4("model", object->GetModelMatrix());
                        if (mesh.second.UseElements) glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
                        else glDrawArrays(GL_TRIANGLES, 0, mesh.second.TriangleCount * 3);
                    }
                    index++;
                }
            }
        }
    }

    Object* GetObjectFromNode(SceneNode* node)
    {
        if (auto object = dynamic_cast<Object*>(node)) {
            return object;
        }
        return nullptr;
    }

    Light* GetLightFromNode(SceneNode* node)
    {
        if (auto light = dynamic_cast<Light*>(node)) {
            return light;
        }
        return nullptr;
    }

    void DrawLights()
    {
        for (auto &node : SM::SceneNodes)
        {
            if (node->GetType() == NodeType::Light_)
            {
                SM::Light* light = dynamic_cast<SM::Light*>(node);   
                qk::DrawScreenAlignedPlane(light->GetPosition(), 0.125f, 1.0f, light->GetColor());
            }
        }
    }

    void DrawOrigin()
    {
        glDisable(GL_DEPTH_TEST);
        qk::DrawScreenAlignedPlane(SceneNodes[GetSelectedIndex()]->GetPosition(), 0.025f, 1.0f, qk::HexToRGB(0xff9f2c));
        glEnable(GL_DEPTH_TEST);
    }

    Object::Object(std::string Name, std::string MeshID)
    {
        _name          = Name;
        _position      = glm::vec3(0.0f);
        _scale         = glm::vec3(1.0f);
        _rotationEuler = glm::vec3(0.0f);
        _rotationQuat  = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        _modelMatrix   = glm::mat4(1.0f);
        _meshID        = MeshID;

        RecalculateMat4();
    }

    void Object::SetPosition(glm::vec3 Position)
    {
        _position = Position;
        RecalculateMat4();
    }

    void Object::SetRotationEuler(glm::vec3 Rotation)
    {
        _rotationEuler = Rotation;
        _rotationQuat  = glm::quat(glm::radians(_rotationEuler));
        RecalculateMat4();
    }

    void Object::SetRotationQuat(glm::quat Rotation)
    {
        _rotationQuat  = Rotation;
        _rotationEuler = glm::degrees(glm::eulerAngles(_rotationQuat));
        RecalculateMat4();
    }

    void Object::Rotate(glm::vec3 Rotation)
    {
        glm::quat deltaQuat = glm::quat(glm::radians(Rotation));
        _rotationQuat = deltaQuat * _rotationQuat;
        _rotationEuler = glm::degrees(glm::eulerAngles(_rotationQuat));

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

        _modelMatrix = glm::translate(identity, _position);
        _modelMatrix *= glm::mat4_cast(_rotationQuat);
        _modelMatrix = glm::scale(_modelMatrix, _scale);

        // _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotationEuler.x), glm::vec3(1.0f, 0.0f, 0.0f));
        // _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotationEuler.y), glm::vec3(0.0f, 1.0f, 0.0f));
        // _modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotationEuler.z), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    void Object::SetName(std::string Name)
    {
        _name = Name;
    }

    glm::vec3 Object::GetPosition()
    {
        return _position;
    }

    glm::vec3 Object::GetRotationEuler()
    {
        return _rotationEuler;
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

    NodeType Object::GetType()
    {
        return _nodeType;
    }

    void CalculateObjectsTriCount()
    {
        int numtris = 0;
        for (size_t i = 0; i < SceneNodes.size(); i++)
        {
            Object* object = dynamic_cast<Object*>(SceneNodes[i]);
            if (object) numtris += AM::Meshes.at(object->GetMeshID()).TriangleCount;
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

    void Light::SetName(std::string Name)
    {
        _name = Name;
    }

    glm::vec3 Light::GetPosition()
    {
        return _position;
    }
    
    glm::vec3 Light::GetColor()
    {
        return _color;
    }

    std::string Light::GetName()
    {
        return _name;
    }

    NodeType Light::GetType()
    {
        return _nodeType;
    }

    float Light::GetIntensity()
    {
        return _intensity;
    }
}
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene_manager.h"
#include "render_engine.h"
#include "asset_manager.h"
#include "../common/camera.h"
#include "../common/qk.h"
#include "../common/input.h"
#include "../ui/ui.h"

namespace SM
{
    int _selectedSceneNode;

    void DrawLights();
    void DrawOrigin();
    void NodeSelection();

    void Initialize()
    {
        Engine::RegisterEditorDrawUIFunction(DrawLights);
        Engine::RegisterEditorDrawUIFunction(DrawOrigin);
        Engine::RegisterEditorFunction(NodeSelection);
    }

    void AddNode(Object* Object)
    {
        SceneNodes.push_back(Object);
        SceneNodeNames.push_back(Object->GetName());
        CalculateObjectsTriCount();

        NumObjects++;
        _selectedSceneNode = SceneNodes.size() - 1;
        printf("Added object \"%s\" with mesh \"%s\"\n", Object->GetName().c_str(), Object->GetMeshID().c_str());
    }

    void AddNode(Light* Light)
    {
        SceneNodes.push_back(Light);
        SceneNodeNames.push_back(Light->GetName());

        NumLights++;
        _selectedSceneNode = SceneNodes.size() - 1;
        printf("Added light \"%s\"\n", Light->GetName().c_str());
    }

    void SelectSceneNode(int Index)
    {
        _selectedSceneNode = glm::min(Index, (int)SceneNodes.size() - 1);
    }

    int GetSelectedIndex()
    {
        return _selectedSceneNode;
    }

    void FocusSelection(float screenPercentage)
    {
        SM::SceneNode* node = SM::SceneNodes[SM::GetSelectedIndex()];
        if (node->GetType() == SM::NodeType::Object_) {
            SM::Object* obj = SM::GetObjectFromNode(node);
            AM::Mesh& mesh = AM::Meshes.at(obj->GetMeshID());
            AM::BVH& bvh = mesh.bvh;

            // Object's model matrix
            glm::mat4 modelMatrix = obj->GetModelMatrix();

            // Get the AABB corners in local space
            glm::vec3 aabbMin = bvh.bvhNodes[bvh.rootIdx].aabb.min;
            glm::vec3 aabbMax = bvh.bvhNodes[bvh.rootIdx].aabb.max;

            // Transform the AABB corners by the full model matrix
            glm::vec3 transformedMin = glm::vec3(modelMatrix * glm::vec4(aabbMin, 1.0f));
            glm::vec3 transformedMax = glm::vec3(modelMatrix * glm::vec4(aabbMax, 1.0f));

            float objectHeight = glm::abs(transformedMax.y - transformedMin.y);
            float objectWidth  = glm::abs(transformedMax.x - transformedMin.x);

            objectHeight /= screenPercentage;
            objectWidth  /= screenPercentage;

            float aspect = Engine::GetWindowSize().x / Engine::GetWindowSize().y;

            // Compute the required distance based on FOV (height or width constraint)
            float distY = (objectHeight * 0.5f) / tan(glm::radians(AM::EditorCam.Fov) * 0.5f);
            float fovX_rad = 2 * atan(tan(glm::radians(AM::EditorCam.Fov) * 0.5f) * aspect);
            float distX = (objectWidth * 0.5f) / tan(fovX_rad * 0.5f);

            // Final camera distance
            float cameraDistance = std::max(distY, distX);

            // Get the camera's view direction
            glm::vec3 viewDirection = glm::normalize(obj->GetPosition() - AM::EditorCam.Position);

            // Set the final camera position
            glm::vec3 cameraPos = obj->GetPosition() - viewDirection * cameraDistance;
            AM::EditorCam.SetTargetPosition(cameraPos);
            AM::EditorCam.SetDirection(viewDirection);
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
                glm::vec2 pos = qk::WorldToScreen(light->GetPosition());
                UI::DrawQuad(pos, 20, light->GetColor());
            }
        }
    }

    void DrawOrigin()
    {
        if (SceneNodes.size() > 0) {
            SceneNode* node = SceneNodes[GetSelectedIndex()];
            glm::vec2 pos = qk::WorldToScreen(node->GetPosition());
            UI::DrawCircle(pos, qk::HexToRGB(0xff9f2c), 5);
        }
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

    void NodeSelection()
    {
        float closestT = FLT_MAX;
        int   closestNodeIndex = -1;
        if (Input::MouseButtonPressed(GLFW_MOUSE_BUTTON_1) && Input::GetInputContext() == Input::Game)
        {
            glm::vec2 mousePos  = glm::vec2(Input::GetMouseX(), Input::GetMouseY());
            glm::vec3 nearPoint = qk::ScreenToWorld(mousePos, 0.0f);
            glm::vec3 farPoint  = qk::ScreenToWorld(mousePos, 1.0f);
            
            glm::vec3 worldDir = glm::normalize(farPoint - nearPoint);
            glm::vec3 worldOrigin = nearPoint;

            AM::ClosestHit closestTri;
            AM::ClosestHit closestAABB;

            int i = 0;
            float lightPickRadius = 0.35f;
            for (const auto& node : SM::SceneNodes) {
                if (node->GetType() == SM::NodeType::Object_)
                {
                    SM::Object* obj  = SM::GetObjectFromNode(node);
                    AM::Mesh&   mesh = AM::Meshes.at(obj->GetMeshID());
                    AM::BVH&    bvh  = mesh.bvh;

                    // Transform ray to object space
                    glm::mat4 modelInv  = glm::inverse(obj->GetModelMatrix());
                    glm::vec3 objOrigin = glm::vec3(modelInv * glm::vec4(worldOrigin, 1.0f));
                    glm::vec3 objDir    = glm::vec3(modelInv * glm::vec4(worldDir, 0.0f));
                    AM::Ray ray(objOrigin, objDir);

                    bvh.TraverseBVH_Ray(bvh.rootIdx, ray, mesh.vertexData, closestTri, closestAABB, obj->GetModelMatrix());
                    if (closestTri.hit && closestTri.t < closestT) {
                        closestT = closestTri.t;
                        closestNodeIndex = i;
                    }
                }
                else if (node->GetType() == SM::NodeType::Light_)
                {
                    SM::Light* light = SM::GetLightFromNode(node);

                    // Ray-sphere intersection
                    glm::vec3 L = light->GetPosition() - worldOrigin;
                    float tca = glm::dot(L, worldDir);
                    if (tca > 0.0f) // Only if light is in front of the camera
                    {
                        float d2 = glm::dot(L, L) - tca * tca;
                        if (d2 <= lightPickRadius * lightPickRadius)
                        {
                            float thc = sqrt(lightPickRadius * lightPickRadius - d2);
                            float t = tca - thc; // distance along ray to closest hit

                            if (t < closestT)
                            {
                                closestT = t;
                                closestNodeIndex = i;
                            }
                        }
                    }
                }
                i++;
            }
        }
        if (closestNodeIndex != -1) SM::SelectSceneNode(closestNodeIndex);
    }
}
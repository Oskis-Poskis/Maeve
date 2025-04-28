#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SM
{
    enum NodeType
    {
        Object_,
        Light_
    };

    enum LightType
    {
        Area,
        Point,
        SpotLight,
        Directional
    };

    class SceneNode
    {
        public:
            virtual ~SceneNode() = default;

            virtual void SetName(std::string Name) = 0;
            virtual std::string GetName() = 0;

            virtual void SetPosition(glm::vec3 Position) = 0;
            virtual glm::vec3 GetPosition() = 0;

            virtual NodeType GetType() = 0;
    };

    class Object : public SceneNode
    {
        public:
            Object(std::string Name, std::string MeshID);

            void SetPosition(glm::vec3 Position);
            void SetRotationEuler(glm::vec3 Rotation);
            void SetRotationQuat(glm::quat Rotation);
            void Rotate(glm::vec3 Rotation);
            void SetScale(glm::vec3 Scale);
            void SetName(std::string Name);
            void RecalculateMat4();
            
            glm::vec3   GetPosition();
            glm::vec3   GetRotationEuler();
            glm::quat   GetRotationQuat();
            glm::vec3   GetScale();
            std::string GetName();
            std::string GetMeshID();
            glm::mat4   &GetModelMatrix();
            NodeType GetType();

        private:
            glm::vec3 _position;
            glm::vec3 _scale;
            glm::vec3 _rotationEuler;
            glm::quat _rotationQuat;
            glm::mat4 _modelMatrix;

            std::string _name;
            std::string _meshID;
            NodeType _nodeType = NodeType::Object_;
    };

    class Light : public SceneNode
    {
        public:
            Light(std::string Name, LightType Type);
            Light(std::string Name, LightType Type, glm::vec3 Position, float Radius, glm::vec3 Color, float Intensity);

            void SetPosition(glm::vec3 Position);
            void SetColor(glm::vec3 Color);
            void SetDirection(glm::vec3 Rotation);

            void  SetIntensity(float Intensity);
            void  SetRadius(float Radius);
            float GetIntensity();

            void SetName(std::string Name);

            glm::vec3 GetPosition();
            glm::vec3 GetColor();
            std::string GetName();
            NodeType GetType();

        private:
            glm::vec3 _position;
            float     _radius    = 1.0f;
            glm::vec3 _color     = glm::vec3(1.0f);
            float     _intensity = 1.0f;
            
            std::string _name;
            LightType   _type;
            NodeType _nodeType = NodeType::Light_;
    };
    
    void RenderAll();
    void CalculateObjectsTriCount();
    void AddNode(Object* Object);
    void AddNode(Light* Object);
    Object* GetObjectFromNode(SceneNode* node);
    Light* GetLightFromNode(SceneNode* node);

    void Initialize();

    void SelectSceneNode(int Index);
    int  GetSelectedIndex();

    inline std::vector<SceneNode*> SceneNodes;
    inline std::vector<std::string> SceneNodeNames;

    inline int ObjectsTriCount;
    inline int NumObjects;
    inline int NumLights;
};
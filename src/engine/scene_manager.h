#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SceneManager
{
    class Object
    {
        public:
            Object(std::string Name, std::string MeshID);

            void SetPosition(glm::vec3 Position);
            void SetRotation(glm::vec3 Rotation);
            void SetScale(glm::vec3 Scale);
            void SetName(std::string Name);
            void RecalculateMat4();
            
            glm::vec3   GetPosition();
            glm::vec3   GetRotation();
            glm::vec3   GetScale();
            std::string GetName();
            std::string GetMeshID();
            glm::mat4   &GetModelMatrix();

        private:
            glm::vec3 _position;
            glm::vec3 _rotation;
            glm::vec3 _scale;
            glm::mat4 _modelMatrix;

            std::string _name;
            std::string _meshID;
    };

    enum LightType
    {
        Area,
        Point,
        SpotLight,
        Directional
    };

    class Light
    {
        public:
            Light(std::string Name, LightType Type);
            Light(std::string Name, LightType Type, glm::vec3 Position, float Radius, glm::vec3 Color, float Intensity);

            void SetPosition(glm::vec3 Position);
            void SetColor(glm::vec3 Color);
            void SetDirection(glm::vec3 Rotation);

            void SetIntensity(float Intensity);
            void SetRadius(float Radius);

            void SetName(std::string Name);

            glm::vec3 GetPosition();
            glm::vec3 GetColor();
            float     GetIntensity();

        private:
            glm::vec3 _position;
            float     _radius    = 1.0f;
            glm::vec3 _color     = glm::vec3(1.0f);
            float     _intensity = 1.0f;
            
            std::string _name;
            LightType   _type;
    };

    void DrawEditorGeometry();
    
    void RenderAll();
    void CalculateObjectsTriCount();
    void AddObject(Object Object);
    void AddLight(Light Light);

    inline std::vector<Light>  Lights;
    inline std::vector<Object> Objects;
    inline std::vector<std::string> ObjectNames;
    inline int ObjectsTriCount;
};
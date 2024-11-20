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
            void SetModelMatrix(glm::vec3 PosOffset, glm::vec3 RotOffset, glm::vec3 ScaleOffset);
            
            glm::vec3   GetPosition();
            glm::vec3   GetRotation();
            glm::vec3   GetScale();
            std::string GetName();
            std::string GetMeshID();
            glm::mat4   GetModelMatrix();

        private:
            glm::mat4 _modelMatrix;
            glm::vec3 _position;
            glm::vec3 _rotation;
            glm::vec3 _scale;

            std::string _name;
            std::string _meshID;
    };

    inline std::vector<Object>      Objects;
    inline std::vector<std::string> ObjectNames;
    inline int ObjectsTriCount;

    void CalculateObjectsTriCount();
    void AddObject(Object Object);
    void RenderAll();
};
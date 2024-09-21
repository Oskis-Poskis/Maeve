#pragma once

#include <string>
#include <vector>

#include "../../lib/glm/glm.hpp"
#include "../../lib/glm/gtc/matrix_transform.hpp"

namespace SceneManager
{
    class Object
    {
        public:
            std::string MeshID;
        
            Object(std::string Name, std::string MeshID);

            void SetPosition(glm::vec3 Position);
            void SetRotation(glm::vec3 Rotation);
            void SetScale(glm::vec3 Scale);
            void SetName(std::string Name);
            void SetModelMatrix();
            
            glm::vec3 GetPosition();
            glm::vec3 GetRotation();
            glm::vec3 GetScale();
            std::string GetName();
            glm::mat4 GetModelMatrix();

        private:
            glm::mat4 _modelMatrix;
            glm::vec3 _position;
            glm::vec3 _rotation;
            glm::vec3 _scale;

            std::string _name;
    };

    inline std::vector<Object> Objects;
    inline int ObjectsTriCount;
    void CalculateObjectsTriCount();
    void AddObject(Object Object);
    void RenderAll(float fov, glm::mat4 vMat, glm::vec3 cPos);
};
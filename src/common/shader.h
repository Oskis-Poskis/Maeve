#pragma once

#include <string>
#include <glm/gtc/matrix_transform.hpp>

class Shader
{
    public:
        unsigned int ID;

        Shader(std::string Path);

        void Use();
        void Reload();
        void SetBool(const std::string &name, bool value) const;
        void SetInt(const std::string &name, int value) const;
        void SetFloat(const std::string &name, float value) const;
        void SetVector3(const std::string &name, glm::vec3 value) const;
        void SetMatrix4(const std::string &name, glm::mat4 value) const;
    
    private:
        void _createShader();
        std::string _shaderPath;
};
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Movement_Direction
{
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
    DOWN,
    UP,
};

const float YAW   = -90.0f;
const float PITCH =  0.0f;
const float SPEED =  10.0f;
const float FOV   =  50.0f;
const float INTERPOLATION_MULTIPLIER = 0.1f;
const float SENSITIVITY = 0.125f;

class Camera
{
    public:
        glm::vec3 Position;
        glm::vec3 WorldUp;
        glm::vec3 Up;
        glm::vec3 Front;
        glm::vec3 Right;

        float Yaw;
        float Pitch;
        float Speed;
        float Fov;
        float InterpolationMultiplier;
        float Sensitivity;
        bool  Moving;
        bool  Turning;

        Camera();
        Camera(glm::vec3 position, float fov, float yaw, float pitch);
        
        glm::mat4 GetViewMatrix();
        void MouseInput(float xOffset, float yOffset);
        void KeyboardInput(Movement_Direction direction);
        void Update();

    private:
        glm::vec3 _targetPosition;
        void _updateCameraVectors();
};
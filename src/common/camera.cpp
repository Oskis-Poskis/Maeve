#include <iostream>

#include <GLFW/glfw3.h>

#include "qk.h"
#include "camera.h"
#include "input.h"
#include "stat_counter.h"
#include "../engine/render_engine.h"
#include "../engine/asset_manager.h"

Camera::Camera() : Position(glm::vec3(0.0f, 0.0f, 0.0f)),
                   _targetPosition(Position),
                   Fov(FOV),
                   Yaw(YAW),
                   Pitch(PITCH),
                   Front(glm::vec3(0.0f, 1.0f, 0.0f)),
                   WorldUp(glm::vec3(0.0f, 0.0f, 1.0f)),
                   Speed(SPEED),
                   Sensitivity(SENSITIVITY),
                   InterpolationMultiplier(INTERPOLATION_MULTIPLIER)
{
    _updateCameraVectors();
}

Camera::Camera(glm::vec3 position, float fov, float yaw, float pitch) : Front(glm::vec3(0.0f, 1.0f, 0.0f)),
                                                                        WorldUp(glm::vec3(0.0f, 0.0f, 1.0f)),
                                                                        Speed(SPEED),
                                                                        Sensitivity(SENSITIVITY),
                                                                        InterpolationMultiplier(INTERPOLATION_MULTIPLIER)
{
    Position        = position;
    _targetPosition = position;
    Yaw             = yaw;
    Pitch           = pitch;
    Fov             = fov;

    _updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::MouseInput(float xoffset, float yoffset)
{
    xoffset *= Sensitivity;
    yoffset *= Sensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    Yaw   = glm::mod(Yaw, 360.0f);
    Pitch = glm::clamp(Pitch, -89.99f, 89.99f);
    
    _updateCameraVectors();
}

void Camera::KeyboardInput(Movement_Direction direction)
{
    float velocity = Speed * Stats::GetDeltaTime();
    if (direction == FORWARD)  _targetPosition += Front   * velocity;
    if (direction == BACKWARD) _targetPosition -= Front   * velocity;
    if (direction == RIGHT)    _targetPosition += Right   * velocity;
    if (direction == LEFT)     _targetPosition -= Right   * velocity;
    if (direction == DOWN)     _targetPosition -= WorldUp * velocity;
    if (direction == UP)       _targetPosition += WorldUp * velocity;
}

void Camera::SetPosition(glm::vec3 NewPosition)
{
    Position        = NewPosition;
    _targetPosition = NewPosition;
}

void Camera::SetTargetPosition(glm::vec3 NewPosition)
{
    _targetPosition = NewPosition;
}

void Camera::SetDirection(glm::vec3 NewDirection)
{
    Front = glm::normalize(NewDirection);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));

    Pitch = glm::degrees(asin(Front.z));
    Yaw  = -glm::degrees(atan2(Front.y, Front.x));
}

bool firstClick = true, unrightclick = true;
double mousex, mousey;
double downposx, downposy;

void Camera::Update()
{
    if (Input::GetInputContext() != Input::TextInput)
    {
        if (Input::MouseButtonPressed(GLFW_MOUSE_BUTTON_2) || Input::KeyDown(GLFW_KEY_LEFT_ALT))
        {
            Turning = true;
            glfwSetInputMode(Engine::WindowPtr(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            if (firstClick)
            {
                glfwGetCursorPos(Engine::WindowPtr(), &downposx, &downposy);
                glfwSetCursorPos(Engine::WindowPtr(), Engine::GetWindowSize().x / 2, Engine::GetWindowSize().y / 2);
                firstClick = false;
                unrightclick = false;
            }

            glfwGetCursorPos(Engine::WindowPtr(), &mousex, &mousey);
            float dy = (float)(mousey - (Engine::GetWindowSize().y / 2));
		    float dx = (float)(mousex - (Engine::GetWindowSize().x / 2));
            MouseInput(dx, -dy);
            
            glfwSetCursorPos(Engine::WindowPtr(), Engine::GetWindowSize().x / 2, Engine::GetWindowSize().y / 2);
        }
        else
        {
            Turning = false;
            glfwSetInputMode(Engine::WindowPtr(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            if (!unrightclick)
            {
                glfwSetCursorPos(Engine::WindowPtr(), downposx, downposy);
                unrightclick = true;
            }
            firstClick = true;
        }

        if (!Input::KeyDown(GLFW_KEY_LEFT_SHIFT))
        {
            Moving = true;

            if (!(Input::KeyDown(GLFW_KEY_W) || Input::KeyDown(GLFW_KEY_S) ||
                  Input::KeyDown(GLFW_KEY_A) || Input::KeyDown(GLFW_KEY_D) ||
                  Input::KeyDown(GLFW_KEY_E) || Input::KeyDown(GLFW_KEY_Q)))
            {
                Moving = false;
            }
            else {
                if (Input::KeyDown(GLFW_KEY_W)) KeyboardInput(FORWARD);
                if (Input::KeyDown(GLFW_KEY_S)) KeyboardInput(BACKWARD);
                if (Input::KeyDown(GLFW_KEY_A)) KeyboardInput(LEFT);
                if (Input::KeyDown(GLFW_KEY_D)) KeyboardInput(RIGHT);
                if (Input::KeyDown(GLFW_KEY_E)) KeyboardInput(UP);
                if (Input::KeyDown(GLFW_KEY_Q)) KeyboardInput(DOWN);
            }
        }

        Position = glm::mix(Position, _targetPosition, InterpolationMultiplier * Stats::GetDeltaTime());
        // Position = _targetPosition;
    }
}

void Camera::_updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(-Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(-Yaw)) * cos(glm::radians(Pitch));
    front.z = sin(glm::radians(Pitch));

    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
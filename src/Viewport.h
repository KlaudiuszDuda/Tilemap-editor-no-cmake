#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "types.h"
#include "Window.h"

class Viewport
{
public:
    Viewport(glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
    Viewport(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	inline glm::mat4 getView();
	inline glm::mat4 getProjection();
	inline glm::mat4 getViewAndProjection();

	inline void lockMouse();
    inline void updateCameraVectors();

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

	f32 Yaw;
	f32 Pitch;
	f32 Fov;
	f32 Sensitivity;

    f32 LastX = 0.f;
    f32 LastY = 0.f;
};

glm::mat4 Viewport::getView()
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Viewport::getProjection()
{
    return glm::perspective(glm::radians(Fov), window.getWindowSize().x / window.getWindowSize().y, 0.1f, 100000.0f);
}

glm::mat4 Viewport::getViewAndProjection()
{
    return glm::perspective(glm::radians(Fov), window.getWindowSize().x / window.getWindowSize().y, 0.1f, 100000.0f) * glm::lookAt(Position, Position + Front, Up);
}

inline void Viewport::lockMouse()
{
    float xoffset = window.getMousePosition().x - LastX;
    float yoffset = LastY - window.getMousePosition().y;
    LastX = window.getMousePosition().x;
    LastY = window.getMousePosition().y;

    xoffset *= Sensitivity;
    yoffset *= Sensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (Pitch > 89.0f)
        Pitch = 89.0f;
    if (Pitch < -89.0f)
        Pitch = -89.0f;

    updateCameraVectors();
}

inline void Viewport::updateCameraVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}
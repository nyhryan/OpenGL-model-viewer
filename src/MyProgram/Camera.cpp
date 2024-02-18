#include "Camera.h"

#include <gl/gl3w.h>
#include <glfw/glfw3.h>

#include <fmt/core.h>

Camera::Camera(int screenW, int screenH)
{
    glm::vec3 front;
    front.x = glm::cos(glm::radians(yawAngle)) * glm::cos(glm::radians(pitchAngle));
    front.y = glm::sin(glm::radians(pitchAngle));
    front.z = glm::sin(glm::radians(yawAngle)) * glm::cos(glm::radians(pitchAngle));
    z = glm::normalize(front);
    z *= -1.0f;

    x = glm::cross(glm::vec3(0.0, 1.0, 0.0), z);
    y = glm::cross(z, x);

    x = glm::normalize(x);
    y = glm::normalize(y);

    // z = glm::vec3(0.0, 0.0, 1.0);
    // x = glm::vec3(1.0, 0.0, 0.0);
    // y = glm::vec3(0.0, 1.0, 0.0);

    lastMouseX = screenW / 2.0f;
    lastMouseY = screenH / 2.0f;
}

void Camera::UpdateCameraVecs()
{
    glm::vec3 front;
    front.x = glm::cos(glm::radians(yawAngle)) * glm::cos(glm::radians(pitchAngle));
    front.y = glm::sin(glm::radians(pitchAngle));
    front.z = glm::sin(glm::radians(yawAngle)) * glm::cos(glm::radians(pitchAngle));
    z = glm::normalize(front);
    z *= -1.0f;

    x = glm::cross(glm::vec3(0.0, 1.0, 0.0), z);

    y = glm::cross(z,x);

    x = glm::normalize(x);
    y = glm::normalize(y);
}

void Camera::ProcessMouse(float xOffset, float yOffset)
{
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yawAngle += xOffset;
    pitchAngle += yOffset;

    if (pitchAngle > 89.0f)
        pitchAngle = 89.0f;
    if (pitchAngle < -89.0f)
        pitchAngle = -89.0f;

    // fmt::print("yaw: {}, pitch: {}\n", yawAngle, pitchAngle);

    UpdateCameraVecs();
}
#pragma once

#include <glm/glm.hpp>

#include <fmt/core.h>

class Camera
{
public:
    Camera() {}
    Camera(int, int);
    ~Camera() = default;

public:
    void MoveForward(float dt) { pos -= z * dt * moveSpeed; }
    void MoveBackward(float dt) { pos += z * dt * moveSpeed; }
    void MoveLeft(float dt) { pos -= x * dt * moveSpeed; }
    void MoveRight(float dt) { pos += x * dt * moveSpeed; }
    void Ascend(float dt) { pos += y * dt * moveSpeed; }
    void Descend(float dt) { pos -= y * dt * moveSpeed; }

    void ProcessMouse(float, float);

public:
    const glm::vec3 GetPos() const { return pos; }

    const glm::vec3 GetZ() const { return z; }
    void SetZ(const glm::vec3& front) { this->z = front; }

    const glm::vec3 GetX() const { return x; }
    const glm::vec3 GetY() const { return y; }

    const float GetMoveSpeed() const { return moveSpeed; }
    void SetMoveSpeed(float spd) { moveSpeed = spd; }
    
    const float GetFov() const { return fov; }
    void SetFov(float fov) { this->fov = fov; }

    const bool GetFirstCapture() const { return isFirstCapture; }
    void SetFirstCapture(bool isFirstCapture) { this->isFirstCapture = isFirstCapture; }

    const float GetMouseSensitivity() const { return mouseSensitivity; }
    void SetMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }

    const float GetLastMouseX() { return lastMouseX; }
    void SetLastMouseX(float x) { lastMouseX = x; }
    const float GetLastMouseY() { return lastMouseY; }
    void SetLastMouseY(float y) { lastMouseY = y; }

private:
    void UpdateCameraVecs();

private:
    glm::vec3 pos = glm::vec3(0.0f, 3.0f, 10.0f);
    glm::vec3 z;
    glm::vec3 x; // camera's right vector
    glm::vec3 y; // camera's up vector
    float moveSpeed = 0.1f;
    float fov = 45.0f;

    bool isFirstCapture = true;
    float mouseSensitivity = 1.0f;
    float lastMouseX;
    float lastMouseY;
    float pitchAngle = 0.0f;
    float yawAngle = -90.0f;
};
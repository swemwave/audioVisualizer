#pragma once

#include <DirectXMath.h>
#include <windows.h>

class Camera {
private:
    // Matrices
    DirectX::XMMATRIX viewMatrix;
    DirectX::XMMATRIX projectionMatrix;

    // Camera position and orientation
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 rotation; // Pitch, Yaw, Roll in radians

    // Camera properties
    float movementSpeed;
    float rotationSpeed;
    float fieldOfView;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    // Mouse state tracking
    bool mouseLookEnabled;
    int lastMousePosX;
    int lastMousePosY;

    // Update matrices when position or rotation changes
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

public:
    Camera();
    ~Camera();

    // Initialize camera
    void Initialize(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);

    // Update camera (process input, etc.)
    void Update(float deltaTime);

    // Handle input
    void ProcessKeyboard(float deltaTime);
    void ProcessMouse(int mouseX, int mouseY);

    // Toggle mouse looking
    void EnableMouseLook(bool enable);

    // Matrix access
    DirectX::XMMATRIX GetViewMatrix() const { return viewMatrix; }
    DirectX::XMMATRIX GetProjectionMatrix() const { return projectionMatrix; }

    // Camera position access
    DirectX::XMFLOAT3 GetPosition() const { return position; }
    void SetPosition(float x, float y, float z);

    // Camera rotation access (in radians)
    DirectX::XMFLOAT3 GetRotation() const { return rotation; }
    void SetRotation(float pitch, float yaw, float roll);

    // Update aspect ratio (for window resizing)
    void SetAspectRatio(float newAspectRatio);
};
#include "Camera.h"
#include <iostream>

// For keyboard input detection
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

Camera::Camera()
    : position(0.0f, 0.0f, -5.0f),
    rotation(0.0f, 0.0f, 0.0f),
    movementSpeed(5.0f),
    rotationSpeed(0.005f),
    fieldOfView(DirectX::XM_PIDIV4), // 45 degrees in radians
    aspectRatio(1.0f),
    nearPlane(0.1f),
    farPlane(1000.0f),
    mouseLookEnabled(false),
    lastMousePosX(0),
    lastMousePosY(0)
{
    // Initialize matrices
    viewMatrix = DirectX::XMMatrixIdentity();
    projectionMatrix = DirectX::XMMatrixIdentity();
}

Camera::~Camera() {
    // Nothing to clean up
}

void Camera::Initialize(float fov, float aspect, float nearZ, float farZ) {
    fieldOfView = fov;
    aspectRatio = aspect;
    nearPlane = nearZ;
    farPlane = farZ;

    // Initialize view and projection matrices
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void Camera::UpdateViewMatrix() {
    // Create rotation matrix from pitch, yaw, roll
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
        rotation.x, rotation.y, rotation.z);

    // Create translation vector and transform to world space
    DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);

    // Calculate camera looking direction (forward vector)
    DirectX::XMVECTOR defaultForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR lookDirection = DirectX::XMVector3TransformCoord(defaultForward, rotationMatrix);

    // Calculate up vector
    DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // Create look-at view matrix
    DirectX::XMVECTOR lookAtPosition = DirectX::XMVectorAdd(positionVector, lookDirection);
    viewMatrix = DirectX::XMMatrixLookAtLH(positionVector, lookAtPosition, upDirection);
}

void Camera::UpdateProjectionMatrix() {
    // Create perspective projection matrix
    projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        fieldOfView,    // Field of view in radians
        aspectRatio,    // Aspect ratio
        nearPlane,      // Near plane distance
        farPlane        // Far plane distance
    );
}

void Camera::Update(float deltaTime) {
    // Process keyboard input
    ProcessKeyboard(deltaTime);

    // Matrices are updated within the input processing functions
    // when position or rotation changes
}

void Camera::ProcessKeyboard(float deltaTime) {
    bool positionChanged = false;
    bool rotationChanged = false;

    // Calculate the movement distance for this frame
    float moveDistance = movementSpeed * deltaTime;
    float rotateAmount = 1.0f * deltaTime;

    // Create the rotation matrix based on current rotation
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
        rotation.x, rotation.y, rotation.z);

    // Define direction vectors
    DirectX::XMVECTOR defaultRight = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR defaultForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR defaultUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // Transform direction vectors by rotation matrix
    DirectX::XMVECTOR rightVector = DirectX::XMVector3TransformCoord(defaultRight, rotationMatrix);
    DirectX::XMVECTOR forwardVector = DirectX::XMVector3TransformCoord(defaultForward, rotationMatrix);
    DirectX::XMVECTOR upVector = DirectX::XMVector3TransformCoord(defaultUp, rotationMatrix);

    // Forward/Backward movement (W/S keys)
    if (KEY_DOWN('W')) {
        DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);
        positionVector = DirectX::XMVectorAdd(positionVector, DirectX::XMVectorScale(forwardVector, moveDistance));
        DirectX::XMStoreFloat3(&position, positionVector);
        positionChanged = true;
    }
    if (KEY_DOWN('S')) {
        DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);
        positionVector = DirectX::XMVectorSubtract(positionVector, DirectX::XMVectorScale(forwardVector, moveDistance));
        DirectX::XMStoreFloat3(&position, positionVector);
        positionChanged = true;
    }

    // Left/Right movement (A/D keys)
    if (KEY_DOWN('A')) {
        DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);
        positionVector = DirectX::XMVectorSubtract(positionVector, DirectX::XMVectorScale(rightVector, moveDistance));
        DirectX::XMStoreFloat3(&position, positionVector);
        positionChanged = true;
    }
    if (KEY_DOWN('D')) {
        DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);
        positionVector = DirectX::XMVectorAdd(positionVector, DirectX::XMVectorScale(rightVector, moveDistance));
        DirectX::XMStoreFloat3(&position, positionVector);
        positionChanged = true;
    }

    // Up/Down movement (E/Q keys)
    if (KEY_DOWN('E')) {
        DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);
        positionVector = DirectX::XMVectorAdd(positionVector, DirectX::XMVectorScale(upVector, moveDistance));
        DirectX::XMStoreFloat3(&position, positionVector);
        positionChanged = true;
    }
    if (KEY_DOWN('Q')) {
        DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);
        positionVector = DirectX::XMVectorSubtract(positionVector, DirectX::XMVectorScale(upVector, moveDistance));
        DirectX::XMStoreFloat3(&position, positionVector);
        positionChanged = true;
    }

    // Arrow key rotation (if mouse look is disabled)
    if (!mouseLookEnabled) {
        // Yaw (Y-axis rotation) - Left/Right arrow keys
        if (KEY_DOWN(VK_LEFT)) {
            rotation.y -= rotateAmount;
            rotationChanged = true;
        }
        if (KEY_DOWN(VK_RIGHT)) {
            rotation.y += rotateAmount;
            rotationChanged = true;
        }

        // Pitch (X-axis rotation) - Up/Down arrow keys
        if (KEY_DOWN(VK_UP)) {
            rotation.x -= rotateAmount;
            rotationChanged = true;
        }
        if (KEY_DOWN(VK_DOWN)) {
            rotation.x += rotateAmount;
            rotationChanged = true;
        }
    }

    // Update view matrix if position or rotation changed
    if (positionChanged || rotationChanged) {
        // Clamp pitch to avoid gimbal lock
        // Using min/max instead of clamp to avoid C++17 dependency
        float minPitch = -DirectX::XM_PIDIV2 + 0.01f;
        float maxPitch = DirectX::XM_PIDIV2 - 0.01f;
        if (rotation.x < minPitch) rotation.x = minPitch;
        if (rotation.x > maxPitch) rotation.x = maxPitch;

        // Update the view matrix
        UpdateViewMatrix();
    }
}

void Camera::ProcessMouse(int mouseX, int mouseY) {
    if (!mouseLookEnabled)
        return;

    // Calculate mouse movement delta
    int dx = mouseX - lastMousePosX;
    int dy = mouseY - lastMousePosY;

    // Update the camera rotation based on mouse movement
    rotation.y += dx * rotationSpeed; // Yaw
    rotation.x += dy * rotationSpeed; // Pitch

    // Clamp pitch to avoid gimbal lock
    // Using min/max instead of clamp to avoid C++17 dependency
    float minPitch = -DirectX::XM_PIDIV2 + 0.01f;
    float maxPitch = DirectX::XM_PIDIV2 - 0.01f;
    if (rotation.x < minPitch) rotation.x = minPitch;
    if (rotation.x > maxPitch) rotation.x = maxPitch;

    // Update view matrix
    UpdateViewMatrix();

    // Store the current mouse position for the next frame
    lastMousePosX = mouseX;
    lastMousePosY = mouseY;
}

void Camera::EnableMouseLook(bool enable) {
    mouseLookEnabled = enable;

    // Get current mouse position
    if (enable) {
        POINT mousePos;
        GetCursorPos(&mousePos);
        lastMousePosX = mousePos.x;
        lastMousePosY = mousePos.y;
    }
}

void Camera::SetPosition(float x, float y, float z) {
    position = DirectX::XMFLOAT3(x, y, z);
    UpdateViewMatrix();
}

void Camera::SetRotation(float pitch, float yaw, float roll) {
    rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
    UpdateViewMatrix();
}

void Camera::SetAspectRatio(float newAspectRatio) {
    aspectRatio = newAspectRatio;
    UpdateProjectionMatrix();
}
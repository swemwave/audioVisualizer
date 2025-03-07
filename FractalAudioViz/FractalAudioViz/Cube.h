#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

class DXRenderer;

class Cube {
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    int vertexCount;
    int indexCount;

    DirectX::XMMATRIX worldMatrix;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 rotation;
    DirectX::XMFLOAT3 scale;

    void UpdateWorldMatrix();

public:
    Cube();
    ~Cube();

    bool Initialize(DXRenderer* renderer);
    void Shutdown();
    void Render(DXRenderer* renderer);

    // Transformation methods
    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z);
    void SetScale(float x, float y, float z);

    // Animation
    void Update(float deltaTime);

    // Getters
    DirectX::XMMATRIX GetWorldMatrix() const { return worldMatrix; }
};
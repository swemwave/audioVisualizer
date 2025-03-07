#include "Cube.h"
#include "DXRenderer.h"

Cube::Cube() :
    vertexCount(0),
    indexCount(0),
    position(0.0f, 0.0f, 0.0f),
    rotation(0.0f, 0.0f, 0.0f),
    scale(1.0f, 1.0f, 1.0f)
{
    worldMatrix = DirectX::XMMatrixIdentity();
}

Cube::~Cube() {
    Shutdown();
}

bool Cube::Initialize(DXRenderer* renderer) {
    ID3D11Device* device = renderer->GetDevice();

    // Define the vertices of the cube (position and color)
    Vertex vertices[] = {
        // Front face
        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
        // Back face
        { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f,  1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f,  1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }
    };

    // Define the indices for the cube
    unsigned short indices[] = {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Back face
        4, 6, 5, 4, 7, 6,
        // Left face
        4, 5, 1, 4, 1, 0,
        // Right face
        3, 2, 6, 3, 6, 7,
        // Top face
        1, 5, 6, 1, 6, 2,
        // Bottom face
        4, 0, 3, 4, 3, 7
    };

    vertexCount = sizeof(vertices) / sizeof(vertices[0]);
    indexCount = sizeof(indices) / sizeof(indices[0]);

    // Create the vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(vertices);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;

    HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
    if (FAILED(result)) {
        return false;
    }

    // Create the index buffer
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(indices);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;

    result = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
    if (FAILED(result)) {
        return false;
    }

    return true;
}

void Cube::SetPosition(float x, float y, float z) {
    position = DirectX::XMFLOAT3(x, y, z);
    UpdateWorldMatrix();
}

void Cube::SetRotation(float x, float y, float z) {
    rotation = DirectX::XMFLOAT3(x, y, z);
    UpdateWorldMatrix();
}

void Cube::SetScale(float x, float y, float z) {
    scale = DirectX::XMFLOAT3(x, y, z);
    UpdateWorldMatrix();
}

void Cube::Update(float deltaTime) {
    // You can add animation or physics logic here if needed
    UpdateWorldMatrix();
}

void Cube::UpdateWorldMatrix() {
    // Create transformation matrices
    DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    DirectX::XMMATRIX rotationX = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotation.x));
    DirectX::XMMATRIX rotationY = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotation.y));
    DirectX::XMMATRIX rotationZ = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rotation.z));
    DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

    // Combine transformations: Scale * Rotation * Translation
    worldMatrix = scaleMatrix * rotationX * rotationY * rotationZ * translationMatrix;
}

void Cube::Render(DXRenderer* renderer) {
    ID3D11DeviceContext* deviceContext = renderer->GetDeviceContext();

    // Set vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* vBuffer = vertexBuffer.Get();
    deviceContext->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);

    // Set index buffer
    deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw indexed
    deviceContext->DrawIndexed(indexCount, 0, 0);
}

void Cube::Shutdown() {
    if (indexBuffer) {
        indexBuffer->Release();
        indexBuffer = nullptr;
    }

    if (vertexBuffer) {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }
}
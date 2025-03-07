#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h> // For ComPtr
#include "Cube.h"

// Link the necessary libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Forward declare Camera class
class Camera;

// Vertex structure for 3D rendering
struct Vertex {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT4 Color;
};

// Constant buffer structure for matrices
struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
};

class DXRenderer {
private:
    // DirectX device resources
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

    // Constant buffer for matrices
    Microsoft::WRL::ComPtr<ID3D11Buffer> matrixBuffer;

    // World matrix for object transformation
    DirectX::XMMATRIX worldMatrix;

    // Window properties
    HWND hwnd;
    int width;
    int height;
    bool vsync;

public:
    DXRenderer();
    ~DXRenderer();

    // Initialize DirectX 11
    bool Initialize(HWND hwnd, int width, int height, bool vsync = true);

    // Release resources
    void Shutdown();

    // Resize the swap chain when the window is resized
    bool ResizeBuffers(int width, int height);

    // Begin and end rendering frame
    void BeginFrame(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
    void EndFrame();

    // Create and set up shaders and input layout
    bool CreateBasicShaders();

    // Create constant buffers
    bool CreateConstantBuffers();

    // Set matrices for rendering
    void SetMatrices(const DirectX::XMMATRIX& world, const Camera* camera);

    // Access device and context
    ID3D11Device* GetDevice() const { return device.Get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return deviceContext.Get(); }
};
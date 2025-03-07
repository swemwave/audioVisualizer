#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h> // For ComPtr

// Link the necessary libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

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

    // Access device and context
    ID3D11Device* GetDevice() const { return device.Get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return deviceContext.Get(); }
};
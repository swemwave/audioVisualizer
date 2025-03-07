#include "DXRenderer.h"
#include <stdexcept>

DXRenderer::DXRenderer() :
    hwnd(nullptr),
    width(0),
    height(0),
    vsync(true)
{
}

DXRenderer::~DXRenderer() {
    Shutdown();
}

bool DXRenderer::Initialize(HWND windowHandle, int windowWidth, int windowHeight, bool vSync) {
    hwnd = windowHandle;
    width = windowWidth;
    height = windowHeight;
    vsync = vSync;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Feature levels supported
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    // Swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    if (vsync) {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }
    else {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    // Create device, device context, and swap chain
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                    // Use default adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Use hardware rendering
        nullptr,                    // No software device
        createDeviceFlags,          // Debug flag
        featureLevels,              // Supported feature levels
        numFeatureLevels,           // Number of feature levels
        D3D11_SDK_VERSION,          // SDK version
        &swapChainDesc,             // Swap chain description
        swapChain.GetAddressOf(),   // Swap chain
        device.GetAddressOf(),      // Device
        &featureLevel,              // Selected feature level
        deviceContext.GetAddressOf() // Device context
    );

    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create DirectX 11 device and swap chain!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Check if the feature level 11 is supported
    if (featureLevel < D3D_FEATURE_LEVEL_11_0) {
        MessageBox(hwnd, L"DirectX 11 is not supported on this device!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Create the render target view
    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to get back buffer!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    hr = device->CreateRenderTargetView(backBuffer, nullptr, renderTargetView.GetAddressOf());
    backBuffer->Release();
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create render target view!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Create depth stencil buffer and view
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil buffer!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil view!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Create depth stencil state
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
    depthStencilStateDesc.DepthEnable = TRUE;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilStateDesc.StencilEnable = FALSE;

    hr = device->CreateDepthStencilState(&depthStencilStateDesc, depthStencilState.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil state!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Create rasterizer state
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create rasterizer state!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Set the active states
    deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
    deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 1);
    deviceContext->RSSetState(rasterizerState.Get());

    // Set up the viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // Successfully initialized
    return true;
}

void DXRenderer::Shutdown() {
    // Wait for GPU to finish all operations
    if (deviceContext)
        deviceContext->ClearState();

    // Release all DirectX resources in reverse order
    renderTargetView.Reset();
    depthStencilView.Reset();
    depthStencilState.Reset();
    depthStencilBuffer.Reset();
    rasterizerState.Reset();
    swapChain.Reset();
    deviceContext.Reset();
    device.Reset();
}

bool DXRenderer::ResizeBuffers(int newWidth, int newHeight) {
    if (!device || !swapChain || !deviceContext)
        return false;

    // Update dimensions
    width = newWidth;
    height = newHeight;

    // Release render target and depth stencil
    renderTargetView.Reset();
    depthStencilView.Reset();
    depthStencilBuffer.Reset();

    // Resize the swap chain
    HRESULT hr = swapChain->ResizeBuffers(
        1,                      // Buffer count
        width,                  // Width
        height,                 // Height
        DXGI_FORMAT_R8G8B8A8_UNORM, // Format
        0                       // Flags
    );

    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to resize swap chain buffers!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Recreate render target view
    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to get back buffer after resize!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    hr = device->CreateRenderTargetView(backBuffer, nullptr, renderTargetView.GetAddressOf());
    backBuffer->Release();
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create render target view after resize!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Recreate depth stencil texture
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil buffer after resize!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Recreate depth stencil view
    hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil view after resize!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Reset render targets and viewport
    deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    // Set up the viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    deviceContext->RSSetViewports(1, &viewport);

    return true;
}

void DXRenderer::BeginFrame(float r, float g, float b, float a) {
    // Clear the render target and depth stencil
    float clearColor[4] = { r, g, b, a };
    deviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColor);
    deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DXRenderer::EndFrame() {
    // Present the back buffer to the screen
    HRESULT hr = swapChain->Present(vsync ? 1 : 0, 0);
    if (FAILED(hr)) {
        // Handle device lost or other errors
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            MessageBox(hwnd, L"DirectX device was lost or reset!", L"Error", MB_OK | MB_ICONERROR);
            // In a real application, you would want to reinitialize the device here
        }
    }
}
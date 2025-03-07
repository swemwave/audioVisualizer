#include <windows.h>
#include "window.h"
#include <string>
#include <cmath>

// GameTimer implementation
GameTimer::GameTimer() : deltaTime(0.0f), totalTime(0.0f), accumulator(0.0f) {
    Reset();
}

void GameTimer::Reset() {
    lastFrameTime = std::chrono::steady_clock::now();
    currentFrameTime = lastFrameTime;
    deltaTime = 0.0f;
    totalTime = 0.0f;
    accumulator = 0.0f;
}

void GameTimer::Tick() {
    currentFrameTime = std::chrono::steady_clock::now();

    // Calculate delta time in seconds
    deltaTime = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();

    // Cap delta time to avoid spiral of death when debugging
    if (deltaTime > 0.25f) {
        deltaTime = 0.25f;
    }

    lastFrameTime = currentFrameTime;
    totalTime += deltaTime;
    accumulator += deltaTime;
}

float GameTimer::GetDeltaTime() const {
    return deltaTime;
}

float GameTimer::GetTotalTime() const {
    return totalTime;
}

float GameTimer::GetAccumulator() const {
    return accumulator;
}

void GameTimer::ConsumeAccumulatedTime(float amount) {
    accumulator -= amount;
}

// GameWindow implementation
GameWindow::GameWindow() : hwnd(nullptr), hInstance(nullptr), running(false), width(800), height(600) {}

GameWindow::~GameWindow() {
    // Clean up resources
}

LRESULT CALLBACK GameWindow::WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GameWindow* window = nullptr;

    if (uMsg == WM_NCCREATE) {
        // Extract the GameWindow instance from creation parameters
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = static_cast<GameWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

        window->hwnd = hwnd;
    }
    else {
        window = reinterpret_cast<GameWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->WindowProc(hwnd, uMsg, wParam, lParam);
    }
    else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK GameWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // Window has been created
        MessageBox(hwnd, L"DirectX 11 initialization will begin.", L"FractalAudioViz", MB_OK | MB_ICONINFORMATION);
        return 0;

    case WM_SIZE:
        // Window has been resized
        width = LOWORD(lParam);
        height = HIWORD(lParam);

        // Resize DirectX buffers if initialized
        if (width > 0 && height > 0 && renderer.GetDevice()) {
            renderer.ResizeBuffers(width, height);
        }
        return 0;

    case WM_CLOSE:
        // Window is being closed
        running = false;
        return 0;

    case WM_DESTROY:
        // Window is being destroyed
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

bool GameWindow::Initialize(HINSTANCE hInst, int nCmdShow) {
    hInstance = hInst;

    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = GameWindow::WindowProcStatic;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"FractalAudioVizWindowClass";
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, L"Failed to register window class!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Create window
    hwnd = CreateWindowEx(
        0,
        L"FractalAudioVizWindowClass",
        L"FractalAudioViz - DirectX 11",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, this  // Pass 'this' pointer for WM_NCCREATE
    );

    if (!hwnd) {
        MessageBox(nullptr, L"Failed to create window!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Show window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Get the actual client area dimensions
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;

    // Initialize DirectX
    if (!renderer.Initialize(hwnd, width, height)) {
        MessageBox(hwnd, L"Failed to initialize DirectX 11!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Reset timer
    timer.Reset();

    running = true;
    return true;
}

void GameWindow::Run() {
    MSG msg = {};

    // Main game loop
    while (running) {
        // Process all available Windows messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running) {
            break;
        }

        // Update game timer
        timer.Tick();

        // Fixed timestep loop
        while (timer.GetAccumulator() >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP);
            timer.ConsumeAccumulatedTime(FIXED_TIMESTEP);
        }

        // Render frame
        Render();
    }
}

void GameWindow::Update(float deltaTime) {
    // Update game state
    // This is called at fixed intervals (60 times per second)

    // Example: Update the window title with FPS information
    float fps = 1.0f / timer.GetDeltaTime();
    float updateRate = 1.0f / FIXED_TIMESTEP;

    std::wstring title = L"FractalAudioViz - DirectX 11 - FPS: " + std::to_wstring(static_cast<int>(fps)) +
        L" - Update Rate: " + std::to_wstring(static_cast<int>(updateRate)) +
        L" Hz - Time: " + std::to_wstring(timer.GetTotalTime());

    SetWindowText(hwnd, title.c_str());
}

void GameWindow::Render() {
    // Calculate a pulsing blue color based on time
    float blueValue = (sinf(timer.GetTotalTime() * 1.0f) + 1.0f) / 2.0f;

    // Begin frame with a dark blue background
    renderer.BeginFrame(0.0f, 0.0f, blueValue * 0.5f, 1.0f);

    // Here is where you'll add DirectX rendering code for your fractal visualization

    // End frame and present to screen
    renderer.EndFrame();
}

// Standalone function for backward compatibility
bool InitWindow(HINSTANCE hInstance, int nCmdShow) {
    GameWindow* window = new GameWindow();

    if (!window->Initialize(hInstance, nCmdShow)) {
        delete window;
        return false;
    }

    window->Run();

    delete window;
    return true;
}
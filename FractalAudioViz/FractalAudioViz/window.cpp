#include <windows.h>
#include <windowsx.h>
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
GameWindow::GameWindow() : 
    hwnd(nullptr), 
    hInstance(nullptr), 
    running(false), 
    width(800), 
    height(600),
    captureMouse(false)
{}

GameWindow::~GameWindow() {
    // Clean up resources
    renderer.Shutdown();
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

    case WM_RBUTTONDOWN:
        // Right mouse button enables mouse look
        captureMouse = true;
        camera.EnableMouseLook(true);

        // Capture the mouse
        SetCapture(hwnd);
        ShowCursor(FALSE);
        return 0;

    case WM_RBUTTONUP:
        // Right mouse button disables mouse look
        captureMouse = false;
        camera.EnableMouseLook(false);

        // Release the mouse
        ReleaseCapture();
        ShowCursor(TRUE);
        return 0;

    case WM_MOUSEMOVE:
        if (captureMouse) {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            // Update camera view based on mouse movement
            camera.ProcessMouse(xPos, yPos);

            // Reset cursor to center of window to prevent it from leaving the window
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int centerX = (clientRect.right - clientRect.left) / 2;
            int centerY = (clientRect.bottom - clientRect.top) / 2;
            POINT pt = { centerX, centerY };
            ClientToScreen(hwnd, &pt);
            SetCursorPos(pt.x, pt.y);
        }
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
    wc.lpszClassName = L"FractalAudioViz";

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, L"Failed to register window class!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Create the window
    hwnd = CreateWindow(
        L"FractalAudioViz",    // Window class
        L"Fractal Audio Visualizer",  // Window title
        WS_OVERLAPPEDWINDOW,   // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, // Position
        width, height,         // Size
        nullptr,               // Parent window
        nullptr,               // Menu
        hInstance,             // Instance handle
        this                   // Additional data
    );

    if (!hwnd) {
        MessageBox(nullptr, L"Failed to create window!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Initialize DirectX renderer
    if (!renderer.Initialize(hwnd, width, height, true)) {
        MessageBox(hwnd, L"Failed to initialize DirectX renderer!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Initialize the camera
    camera.Initialize(DirectX::XM_PIDIV4, static_cast<float>(width) / static_cast<float>(height), 0.1f, 1000.0f);
    camera.SetPosition(0.0f, 0.0f, -5.0f);

    // Initialize the cube
    if (!cube.Initialize(&renderer)) {
        MessageBox(hwnd, L"Failed to initialize cube!", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Position the cube in front of the camera
    cube.SetPosition(0.0f, 0.0f, 0.0f);

    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Reset the timer
    timer.Reset();

    // Set running flag
    running = true;

    return true;
}

void GameWindow::Run() {
    MSG msg = {};

    // Main game loop
    while (running) {
        // Handle Windows messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
        }

        if (!running)
            break;

        // Update game timer
        timer.Tick();

        // Fixed time step for logic updates
        while (timer.GetAccumulator() >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP);
            timer.ConsumeAccumulatedTime(FIXED_TIMESTEP);
        }

        // Render the current frame
        Render();
    }
}

void GameWindow::Update(float deltaTime) {
    // Update game logic here
    camera.Update(deltaTime);

    // Update the cube - maybe rotate it slowly
    static float rotationY = 0.0f;
    rotationY += 15.0f * deltaTime; // 15 degrees per second
    if (rotationY > 360.0f) rotationY -= 360.0f;

    cube.SetRotation(0.0f, rotationY, 0.0f);
    cube.Update(deltaTime);
}

void GameWindow::Render() {
    // Clear the back buffer - use a dark blue background
    renderer.BeginFrame(0.0f, 0.0f, 0.2f, 1.0f);

    // Set up world and camera matrices
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
    renderer.SetMatrices(worldMatrix, &camera);

    // Render game objects here
    cube.Render(&renderer);

    // Present the frame
    renderer.EndFrame();
}

// Global function to initialize window
bool InitWindow(HINSTANCE hInstance, int nCmdShow) {
    // Create game window
    static GameWindow gameWindow;

    // Initialize the window
    if (!gameWindow.Initialize(hInstance, nCmdShow)) {
        return false;
    }

    // Run the game loop
    gameWindow.Run();

    return true;
}
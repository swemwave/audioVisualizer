#pragma once
#include <windows.h>
#include <chrono>
#include "DXRenderer.h"
#include "Camera.h"
#include "Cube.h"

// Game timing constants
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f; // 60 updates per second

// Timer class to handle game timing
class GameTimer {
private:
    std::chrono::steady_clock::time_point lastFrameTime;
    std::chrono::steady_clock::time_point currentFrameTime;
    float deltaTime;
    float totalTime;
    float accumulator;
    bool captureMouse;

public:
    GameTimer();
    void Reset();
    void Tick();
    float GetDeltaTime() const;
    float GetTotalTime() const;
    float GetAccumulator() const;
    void ConsumeAccumulatedTime(float amount);
};

// Window class declaration
class GameWindow {
private:
    HWND hwnd;
    HINSTANCE hInstance;
    bool running;
    GameTimer timer;
    int width;
    int height;
    bool captureMouse;
    Camera camera;
    Cube cube;

    // DirectX renderer
    DXRenderer renderer;

    // Private window procedure
    static LRESULT CALLBACK WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    GameWindow();
    ~GameWindow();
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Run();

    // Game loop methods
    void Update(float deltaTime);
    void Render();
};

// Main window initialization function
bool InitWindow(HINSTANCE hInstance, int nCmdShow);
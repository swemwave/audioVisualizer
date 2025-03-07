// FractalAudioViz.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include "window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize our custom window with game loop
    return InitWindow(hInstance, nCmdShow) ? 0 : 1;
}
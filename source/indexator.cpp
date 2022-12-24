#include "headers/fsWindow.h"

#pragma comment(lib, "d3d11.lib")

void HideConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

void ShowConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}

// Main code
int main(int, char**)
{
    HideConsole();   
    fsWindow main_window;
    main_window.work();
    return 0;
}
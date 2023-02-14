#include "headers/fsWindow.h"
#include <stdio.h>
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
    std::cout << boost::filesystem::current_path().string() << std::endl;
    //HideConsole();
    fsWindow main_window("../data", "../index.db");
    main_window.work();
    return 0;
}
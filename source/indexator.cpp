// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs
#include "headers/fsWindow.h"

#pragma comment(lib, "d3d11.lib")

// Main code
int main(int, char**)
{
    vector<string> paths = indexing::find_in_db("georgia.docx", "..\\..\\index.db");
    for (auto& it: paths)
        cout << it << endl;
    fsWindow main_window;
    main_window.work();
    return 0;
}
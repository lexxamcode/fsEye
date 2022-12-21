    #include "indexing.h"
    #include "imgui/imgui.h"
    #include "imgui/imgui_stdlib.h"
    #include "imgui/imgui_impl_win32.h"
    #include "imgui/imgui_impl_dx11.h"
    #include <d3d11.h>
    #include <tchar.h>

    using namespace std;

    // Data
    static ID3D11Device*            g_pd3dDevice = NULL;
    static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
    static IDXGISwapChain*          g_pSwapChain = NULL;
    static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

    // Forward declarations of helper functions
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool CreateDeviceD3D(HWND hWnd)
    {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    void CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
        if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    }

    void CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void CleanupRenderTarget()
    {
        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
    }

    // Forward declare message handler from imgui_impl_win32.cpp
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Win32 message handler
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }

    class fsWindow
    {
        private:
            string request;
            string directory;

            vector<string> languages;
            vector<string> results;
            vector<FVectorMaker*> feature_makers;

            bool show_request_error;
            bool show_results;
            bool show_settings;
            bool done;


            HWND hwnd; // Created window handle
            WNDCLASSEXW wc;
            ImVec4 clear_color; // Background color
        public:
            fsWindow()
            {
                // Load feature_makers
                languages = {"en", "ru"};
                feature_makers = indexing::create_vector_makers(languages);

                // Create application window
                //ImGui_ImplWin32_EnableDpiAwareness();
                wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"fsEye", NULL };
                ::RegisterClassExW(&wc);
                hwnd = ::CreateWindowW(wc.lpszClassName, L"fsEye", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL);

                 // Initialize Direct3D
                if (!CreateDeviceD3D(hwnd))
                {
                     CleanupDeviceD3D();
                    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
                    throw -1;
                }

                // Show the window
                ::ShowWindow(hwnd, SW_SHOWDEFAULT);
                ::UpdateWindow(hwnd);

                // Setup Dear ImGui context
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();

                ImFontConfig font_config;
                font_config.OversampleH = 1; //or 2 is the same
                font_config.OversampleV = 1;
                font_config.PixelSnapH = 1;

                static const ImWchar ranges[] =
                {
                    0x0020, 0x00FF, // Basic Latin + Latin Supplement
                    0x0400, 0x044F, // Cyrillic
                    0,
                };
                ImGuiIO& io = ImGui::GetIO();
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 14.0f, &font_config, ranges);
                //ImGuiIO& io = ImGui::GetIO(); (void)io;
                //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
                //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

                // Setup Dear ImGui style
                ImGui::StyleColorsDark();
                //ImGui::StyleColorsLight();
                //ImGui::StyleColorsClassic();
                // Setup Platform/Renderer backends
                ImGui_ImplWin32_Init(hwnd);
                ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

                // Our state
                show_request_error = false;
                show_settings = false;
                show_results = true;
                clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

                done = false;
            }
            ~fsWindow()
            {
                // Cleanup
                ImGui_ImplDX11_Shutdown();
                ImGui_ImplWin32_Shutdown();
                ImGui::DestroyContext();

                CleanupDeviceD3D();
                ::DestroyWindow(hwnd);
                ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            }
            void work()
            {
                while (!done)
                {
                    // Poll and handle messages (inputs, window resize, etc.)
                    // See the WndProc() function below for our to dispatch events to the Win32 backend.
                    MSG msg;
                    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                    {
                            ::TranslateMessage(&msg);
                            ::DispatchMessage(&msg);
                            if (msg.message == WM_QUIT)
                                done = true;
                    }
                    if (done)
                        break;
                    // Start the Dear ImGui frame
                    ImGui_ImplDX11_NewFrame();
                    ImGui_ImplWin32_NewFrame();
                    ImGui::NewFrame();

                    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
                    //if (show_demo_window)
                        //ImGui::ShowDemoWindow(&show_demo_window);

                    // SEARCH WINDOW
                    {
                        ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
                        ImGui::Begin("fsEye");   // Create a window called "fsEye" and append into it.

                        ImGui::InputText("##", &request);               // Display some text (you can use a format strings too)
                        //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                        //ImGui::Checkbox("Another Window", &show_another_window);
                        ImGui::SameLine();
                        if (ImGui::Button("Search") && request.size())                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        {
                            string arg = request;
                            if (arg.rfind("content:", 0) == 0)
                            {
                                arg.erase(0, 8);
                                results = indexing::knn_algorithm(arg, "..\\..\\index.db", 10, feature_makers);
                                cout << arg << endl;
                                for (auto& it: results)
                                    cout << it << endl;
                            }
                            if (arg.rfind("name:", 0) == 0)
                            {
                                arg.erase(0, 5);
                                results = indexing::find_in_db(arg, "..\\..\\index.db");
                            }
                            else
                            {
                                show_request_error = true;
                            }
                        }
                        ImGui::SameLine();
                        ImGui::Text("found: %d", results.size());

                        if (ImGui::Button("Settings"))
                            show_settings = !show_settings;

                        ImGui::SameLine();
                        ImGui::Checkbox("Show result", &show_results);
                        ImGui::End();
                    }

                    // SETTINGS WINDOW
                    if (show_settings)
                    {   
                        ImGui::Begin("Settings", &show_settings);
                        ImGui::InputText("Directory", &directory);
                        if (ImGui::Button("Index") && directory.size())                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        {
                            indexing::index_directory_to_db(directory, "..\\..\\index.db");
                        }
                        if (ImGui::Button("Index by content") && directory.size())                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        {
                            indexing::index_directory_by_content(directory, "..\\..\\index.db", languages);
                        }
                        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
                        
                        if (ImGui::Button("Close"))
                            show_settings = false;
                        ImGui::End();
                    }

                    // RESULTS WINDOW
                    // if (show_results)
                    // {
                    //     ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
                    //     ImGui::BeginChild("Scrolling");
                    //     if (results.size())
                    //         for (int n = 0; n < results.size(); n++)
                    //             ImGui::Text("%s: Some text", n);
                    //     ImGui::EndChild();
                    // }

                    // ERROR MESSAGE
                    if (show_request_error)
                        {
                        ImGui::Begin("Request error");

                        ImGui::Text("Wrong request parametr.\n Use name:[request] to find file by name,\nUse content:[request] to find file by content");
                        if (ImGui::Button("Close"))
                            show_request_error = false;
                        ImGui::End();
                    }

                   if (results.size())
                   {
                        ImGui::Begin("Results");
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Found files: ");
                        ImGui::BeginChild("Scrolling");
                        for (size_t i = 0; i < results.size(); i++)
                            ImGui::Text("%s", results[i].c_str());
                        ImGui::EndChild();
                        ImGui::End();
                   }

                    // Rendering
                    ImGui::Render();
                    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
                    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
                    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
                    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

                    g_pSwapChain->Present(1, 0); // Present with vsync
                    //g_pSwapChain->Present(0, 0); // Present without vsync
                }
            }
    };    
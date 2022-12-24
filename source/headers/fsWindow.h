#include "indexing.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

using namespace std;

void TextToClipboard(const char *text)
 {
    if (OpenClipboard(0))
    {
        EmptyClipboard();
        char *hBuff = (char *) GlobalAlloc(GMEM_FIXED, strlen(text) + 1);
        strcpy(hBuff, text);
        SetClipboardData(CF_TEXT, hBuff);
        CloseClipboard();
    }
}


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

struct lf_package
{
    vector<FVectorMaker*>* pointer_to_vector_makers;
    vector<string> languages;
    bool* loaded;
};

// Loading vector makers in separate thread
DWORD WINAPI loading_funcition(LPVOID lpParam)
{
    lf_package* info = (lf_package*)lpParam;

    bool* temp_loaded = info->loaded;

    for (auto& it: info->languages)
        cout << it << endl;
    indexing::create_vector_makers_on_heap(info->languages, info->pointer_to_vector_makers);
    cout << "LOADED" << endl;


    // bool loaded is a critical section of window class
    HANDLE load_mutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, (LPCSTR)"load_mutex");

    *temp_loaded = true;

    ReleaseMutex(load_mutex);

    while(true)
    {
        // Cycle thread
    }
    return 0;
}

struct index_parameters
{
    string directory;
    bool indexing;
};

struct content_index_parameters
{
    string directory;
    vector<string> languages;
    vector<FVectorMaker*> feature_makers;
    bool indexing;
};

content_index_parameters ci_parameters;
index_parameters i_parameters;


DWORD WINAPI content_indexing_thread(LPVOID lpParam)
{
    HANDLE index_event = OpenEvent(EVENT_ALL_ACCESS, TRUE, (LPCSTR)"index_content_event");
    HANDLE index_mutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, (LPCSTR)"index_mutex");
    while(true)
    {
        WaitForSingleObject(index_event, INFINITE);

        WaitForSingleObject(index_mutex, INFINITE);
        ci_parameters.indexing = true;
        ReleaseMutex(index_mutex);

        indexing::index_directory_by_content(ci_parameters.directory, "..\\..\\index.db", ci_parameters.languages, ci_parameters.feature_makers);

        WaitForSingleObject(index_mutex, INFINITE);
        ci_parameters.indexing = false;
        ReleaseMutex(index_mutex);
    }
    return 0;
}

DWORD WINAPI indexing_thread(LPVOID lpParam)
{
    HANDLE index_event = OpenEvent(EVENT_ALL_ACCESS, TRUE, (LPCSTR)"index_event");
    HANDLE index_mutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, (LPCSTR)"index_mutex");
    while(true)
    {
        WaitForSingleObject(index_event, INFINITE);

        WaitForSingleObject(index_mutex, INFINITE);
        ci_parameters.indexing = true;
        ReleaseMutex(index_mutex);

        indexing::index_directory_to_db(i_parameters.directory, "..\\..\\index.db");

        WaitForSingleObject(index_mutex, INFINITE);
        ci_parameters.indexing = false;
        ReleaseMutex(index_mutex);
    }
    return 0;
}

struct find_package
{
    bool by_content;
    string request;
    vector<string> found;
    vector<FVectorMaker*> feature_makers;
    bool searching;
};

find_package request_package;

DWORD WINAPI find_thread(LPVOID lpParam)
{
    HANDLE find_event = OpenEvent(EVENT_ALL_ACCESS, TRUE, (LPCSTR)"search_event");
    HANDLE find_mutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, (LPCSTR)"find_mutex");

    while(true)
    {
        WaitForSingleObject(find_event, INFINITE);

        // WaitForSingleObject(find_mutex, INFINITE);
        // cout << GetLastError() << endl;
        // request_package.searching = true;
        // ReleaseMutex(find_mutex);

        if (request_package.by_content)
            indexing::knn_algorithm(request_package.request, "..\\..\\index.db", 10, request_package.feature_makers, request_package.found);
        else
            indexing::find_in_db(request_package.request, "..\\..\\index.db", request_package.found);
        
        // WaitForSingleObject(find_mutex, INFINITE);
        // request_package.searching = false;
        // ReleaseMutex(find_mutex);
    }
    return 0;
}

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
        vector<FVectorMaker*>* feature_makers;

        bool show_request_error;
        bool show_indexing_error;
        bool show_results;
        bool show_settings;
        bool* loaded;
        bool done;

        HANDLE load_mutex;
        HANDLE index_event;
        HANDLE index_content_event;
        HANDLE index_mutex;
        HANDLE find_mutex;
        HANDLE search_event;

        HANDLE loading_thread;
        HANDLE ic_thread_handle;
        HANDLE i_thread_handle;
        HANDLE f_thread_handle;

        HWND hwnd; // Created window handle
        WNDCLASSEXA wc;
        ImVec4 clear_color; // Background color
    public:
        fsWindow()
        {
            index_content_event = CreateEvent(NULL, FALSE, FALSE, (LPCSTR)"index_content_event");
            index_event = CreateEvent(NULL, FALSE, FALSE, (LPCSTR)"index_event");
            search_event = CreateEvent(NULL, FALSE, FALSE, (LPCSTR)"search_event");

            index_mutex = CreateMutex(NULL, FALSE, (LPCSTR)"index_mutex");
            find_mutex = CreateMutex(NULL, FALSE, (LPCSTR)"find_mutex");

            // Load feature_makers in separate thread
            load_mutex = CreateMutex(NULL, FALSE, (LPCSTR)"load_mutex");
            loaded = new bool;
            *loaded = false;
            feature_makers = new vector<FVectorMaker*>;
            
            // Fill package for loading thread
            lf_package load_thread_package;
            languages = {"en", "ru"};
            load_thread_package.languages = languages;
            load_thread_package.pointer_to_vector_makers = feature_makers;
            load_thread_package.loaded = loaded;

            loading_thread = CreateThread(NULL, 0, loading_funcition, &load_thread_package, 0, 0);
            //

            ic_thread_handle = CreateThread(NULL, 0, content_indexing_thread, 0, 0, 0);
            i_thread_handle = CreateThread(NULL, 0, indexing_thread, 0, 0, 0);
            f_thread_handle = CreateThread(NULL, 0, find_thread, 0, 0, 0);

            // Create application window
            //ImGui_ImplWin32_EnableDpiAwareness();
            wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "fsEye", NULL };
            ::RegisterClassExA(&wc);
            wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE("..\\..\\icon.png"));
            hwnd = ::CreateWindowA(wc.lpszClassName, "fsEye", WS_OVERLAPPED | WS_SYSMENU, 100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL);

                // Initialize Direct3D
            if (!CreateDeviceD3D(hwnd))
            {
                    CleanupDeviceD3D();
                ::UnregisterClassA(wc.lpszClassName, wc.hInstance);
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
            show_indexing_error = false;
            show_request_error = false;
            show_settings = false;
            show_results = true;
            clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            ci_parameters.indexing = false;
            i_parameters.indexing = false;
            request_package.searching = false;
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
            ::UnregisterClassA(wc.lpszClassName, wc.hInstance);

            // Terminate threads
            TerminateThread(loading_thread, 0);
            TerminateThread(ic_thread_handle, 0);
            TerminateThread(i_thread_handle, 0);
            TerminateThread(f_thread_handle, 0);

            //free vector<FVectorMaker>

            //free mutex
                delete loaded;
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

                // CHECK IF LANGUAGE PACKAGE LOADED
                

                // SEARCH WINDOW
                {
                    ImGui::SetNextWindowSize(ImVec2(800, 100), ImGuiCond_Always);
                    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
                    ImGui::Begin("fsEye");   // Create a window called "fsEye" and append into it.

                    ImGui::InputText("##", &request);               // Display some text (you can use a format strings too)
                    //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                    //ImGui::Checkbox("Another Window", &show_another_window);
                    ImGui::SameLine();
                    if (ImGui::Button("Search") && request.size())                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    {
                        WaitForSingleObject(index_mutex, INFINITE);
                        if (!ci_parameters.indexing)
                        {
                            string arg = request;
                            if (arg.rfind("content:", 0) == 0)
                            {
                                arg.erase(0, 8);

                                WaitForSingleObject(load_mutex, INFINITE);
                                WaitForSingleObject(find_mutex, INFINITE);
                                cout << "loaded: " << *loaded << endl;
                                cout << "fmakers.size(): " << feature_makers->size() << endl;
                                cout << "searching: " << request_package.searching << endl;
                                if (*loaded && feature_makers->size() && !request_package.searching)
                                {
                                    request_package.feature_makers = *feature_makers;
                                    request_package.request = arg;
                                    request_package.by_content = true;
                                    request_package.found = results;
                                    SetEvent(search_event);
                                }
                                else
                                    show_indexing_error = true;

                                ReleaseMutex(load_mutex);
                                ReleaseMutex(find_mutex);

                                cout << arg << endl;
                                for (auto& it: results)
                                    cout << it << endl;
                            }
                            if (arg.rfind("name:", 0) == 0)
                            {
                                arg.erase(0, 5);
                                //WaitForSingleObject(find_mutex, INFINITE);
                                if (!request_package.searching)
                                {
                                    request_package.request = arg;
                                    request_package.by_content = false;
                                    request_package.found = results;
                                    SetEvent(search_event);
                                }
                                //ReleaseMutex(find_mutex);
                            }
                        }
                        else
                        {
                            show_request_error = true;
                        }
                            // Else Indexing is going on
                        
                    }
                    ImGui::SameLine();
                    ImGui::Text("found: %d", request_package.found.size());

                    if (ImGui::Button("Settings"))
                        show_settings = !show_settings;

                    WaitForSingleObject(load_mutex, INFINITE);
                    if (!(*loaded))
                        ImGui::Text("Loading dictionaries... Please wait and do not use indexation and content search.");
                    ReleaseMutex(load_mutex);

                    WaitForSingleObject(index_mutex, INFINITE);
                        if(ci_parameters.indexing || i_parameters.indexing)
                            ImGui::Text("Indexing... Search and indexing are not available while indexing.");
                    ReleaseMutex(index_mutex);

                     WaitForSingleObject(find_mutex, INFINITE);
                        if(request_package.searching)
                            ImGui::Text("Searching... Please wait.");
                    ReleaseMutex(index_mutex);

                    ImGui::End();
                }

                // SETTINGS WINDOW
                if (show_settings)
                {   
                    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
                    ImGui::SetNextWindowPos(ImVec2(500, 100), ImGuiCond_Always);
                    ImGui::Begin("Settings", &show_settings);
                    ImGui::InputText("Directory", &directory);
                    if (ImGui::Button("Index") && directory.size())                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    {
                        WaitForSingleObject(index_mutex, INFINITE);
                        if (!ci_parameters.indexing && !i_parameters.indexing)
                        {
                            if (*loaded)
                                {
                                    i_parameters.directory = directory;
                                    SetEvent(index_event);
                                }
                            else
                                show_indexing_error = true;
                        }
                        ReleaseMutex(index_mutex);
                    }
                    if (ImGui::Button("Index by content") && directory.size())                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    {
                        WaitForSingleObject(index_mutex, INFINITE);
                        if (!ci_parameters.indexing && !i_parameters.indexing)
                        {
                            WaitForSingleObject(load_mutex, INFINITE);

                            if (languages.size() && *loaded && feature_makers->size())
                            {
                                ci_parameters.directory = directory;
                                ci_parameters.feature_makers = *feature_makers;
                                ci_parameters.languages = languages;
                                SetEvent(index_content_event);
                            }
                            else
                                show_indexing_error = true;

                            ReleaseMutex(load_mutex);
                        
                        }
                        ReleaseMutex(index_mutex);
                    }
                    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
                    
                    if (ImGui::Button("Close"))
                        show_settings = false;
                    ImGui::End();
                }

                
                // REQUEST ERROR MESSAGE
                if (show_request_error)
                {
                    ImGui::Begin("Request error");

                    ImGui::Text("Wrong request parametr.\n Use name:[request] to find file by name,\nUse content:[request] to find file by content");
                    if (ImGui::Button("Close"))
                        show_request_error = false;
                    ImGui::End();
                }
                //NOT LOADED MESSAGE
                if (show_indexing_error)
                {
                    ImGui::Begin("Indexing error");

                    ImGui::Text("Language parameters are not loaded yet. Or indexing is working at the moment\nPlease wait.");
                    if (ImGui::Button("Close"))
                        show_indexing_error = false;
                    ImGui::End();
                }
                // RESULTS WINDOW
                WaitForSingleObject(find_mutex, INFINITE);
                if (request_package.found.size() && !request_package.searching)
                {
                    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Always);
                    ImGui::SetNextWindowPos(ImVec2(0, 100), ImGuiCond_Always);
                    ImGui::Begin("Results");
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Found files: ");
                    ImGui::BeginChild("Scrolling");
                    for (size_t i = 0; i < request_package.found.size(); i++)
                    {
                        if(ImGui::Selectable(request_package.found[i].c_str()) == true)
                        {
                            TextToClipboard(request_package.found[i].c_str());
                        };
                    }
                    ImGui::EndChild();
                    ImGui::End();
                }
                ReleaseMutex(find_mutex);

                // RENDERING
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
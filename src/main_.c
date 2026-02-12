
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>
#include <renderer.h>
#include <plugin_manager_common.h>

bool app_running = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


// TODO: Add error handling popup (look perplexity on how)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)pCmdLine;

#ifdef IS_DEBUG   
    if (AllocConsole())
    {
        FILE *fDummy;

        // Redirect standard streams to the new console "CONOUT$" and "CONIN$"
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);
    }
#endif // #if IS_DEBUG

    printf("Initializing\n");

    HRESULT hr;

    // Initialize COM
    hr = CoInitialize(NULL);

    const wchar_t CLASS_NAME[] = L"MainWindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        CLASS_NAME,
        L"This works?",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hwnd == NULL)
    {
        CoUninitialize();
        return 1;
    }

    renderer_init();

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    TODO("while (app.running)")
    // TODO: while (app.running)
    while (app_running)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                TODO("app.running = false;")
                // TODO: app.running = false;
                app_running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        TODO("if (app.running)")
        // TODO: if (app.running)
        if (app_running)
        {
            TODO("Do rendering stuff")
            // TODO: Do rendering stuff
        }
    }

    CoUninitialize();
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
        ValidateRect(hwnd, NULL);
        return 0;

    case WM_SIZE:
        TODO("Recreate shit!")
        // TODO: Recreate shit!
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
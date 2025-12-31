#define UNICODE
#define _UNICODE
#define COBJMACROS

#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// TODO: Add error handling popup (look perplexity on how)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)pCmdLine;

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
        L"Test window",
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

#warning TODO: Add vulkan init code
    // TODO: Add vulkan init code

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
#warning TODO: while (app.running)
    // TODO: while (app.running)
    while (1)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
#warning TODO: app.running = false
                // TODO: app.running = false
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (1)
#warning TODO: if (app.running)
        // TODO: if (app.running)
        {
#warning TODO: Do render shit
            // TODO: Do render shit
        }
    }

#warning TODO: Do render cleanup
    // TODO: Do render cleanup

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
#warning TODO: Add recreating of stuff here
        // TODO: Add recreating of stuff here
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
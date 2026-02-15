#include "window_win32_plugin.h"

#include <Windows.h>
#include <assert.h>

#include <window_api.h>
#include <environment_api.h>
#include <plugin_manager_common.h>
#include <logger_api.h>
LOGGER_API_REGISTER(window_win32_plugin, LOG_LEVEL_DEBUG);

#include "window_win32_plugin_register.h"

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int32_t window_win32_plugin_create_window(WindowApiContext *context, WindowApiCreateWindowOptions *options)
{
    (void)options;
    WindowsPlatformContext *windows_context;
    ENVIRONMENT_API_GET_WINDOWS_CONTEXT(context->environment_api, &windows_context);

    const wchar_t CLASS_NAME[] = L"MainWindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = window_proc;
    wc.hInstance = windows_context->hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        CLASS_NAME,
        // options->window_name,
        L"Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        windows_context->hInstance,
        context);

    if (hwnd == NULL)
    {
        CoUninitialize();
        return 1;
    }

    // renderer_init();

    ShowWindow(hwnd, windows_context->nCmdShow);
    return 0;
}

int32_t window_win32_plugin_poll_events(WindowApiContext *context)
{
    (void)context;
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            TODO("app.running = false;")
            // TODO: app.running = false;
            // app_running = false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

int32_t window_win32_plugin_wait_for_events(WindowApiContext *context)
{
    return NOT_IMPLEMENTED(int32_t, context);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowApiContext *context = NULL;

    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
        context = (WindowApiContext *)pCreate->lpCreateParams;

        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)context);
    }
    else
    {
        context = (WindowApiContext *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (context == NULL)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // LOG_INF(context->logger_api, "Inside of event");

    switch (uMsg)
    {
    case WM_PAINT:
        ValidateRect(hwnd, NULL);
        return 0;

    case WM_SIZE:
        TODO("Recreate shit!")
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
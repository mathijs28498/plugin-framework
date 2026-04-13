#include "window_win32.h"

#include <Windows.h>
#include <assert.h>

#include <window_interface.h>
#include <environment_interface.h>
#include <plugin_utils.h>
#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(window_win32, LOG_LEVEL_DEBUG);

#include "window_win32_register.h"
#include "window_win32_window_events.h"
#include "window_win32_key_converter.h"

TODO("Add support for multiple screens")

#define WINDOW_WIN32_MAX_OS_EVENTS_PER_FRAME 256

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WindowInterfaceOSHandles window_win32_get_os_handles(WindowContext *context)
{
    assert(context != NULL);
    WindowsPlatformContext *platform_context;

    environment_get_platform_context(context->environment, &platform_context);

    return (WindowInterfaceOSHandles){
        .window_handle = context->hwnd,
        .platform_context_handle = platform_context->hInstance,
    };
}

int32_t window_win32_create_window(WindowContext *context, WindowInterfaceCreateWindowOptions *options)
{
    assert(context != NULL);
    assert(options != NULL);

    TODO("Fix this function")
    WindowsPlatformContext *windows_context;
    environment_get_platform_context(context->environment, &windows_context);

    const wchar_t CLASS_NAME[] = L"MainWindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = window_proc;
    wc.hInstance = windows_context->hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    TODO("Check if this can be done without fixed sized array (I want options->window_name to be a const char *)")
    wchar_t wide_window_name[sizeof(options->window_name)];
    if (MultiByteToWideChar(CP_UTF8, 0, options->window_name, -1, wide_window_name, sizeof(options->window_name)) == 0)
    {
        wide_window_name[0] = L'\0';
    }

    context->hwnd = CreateWindow(
        CLASS_NAME,
        wide_window_name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        windows_context->hInstance,
        context);

    if (context->hwnd == NULL)
    {
        return 1;
    }

    ShowWindow(context->hwnd, windows_context->nCmdShow);
    return 0;
}

int32_t window_win32_get_window_size(WindowContext *context, uint32_t *width, uint32_t *height)
{
    assert(context != NULL);
    assert(width != NULL);
    assert(height != NULL);

    RECT rect;

    RETURN_IF_FALSE(context->logger, GetWindowRect(context->hwnd, &rect),
                    -1, "Failed to get window rect");

    *width = (uint32_t)(rect.right - rect.left);
    *height = (uint32_t)(rect.bottom - rect.top);

    return 0;
}

int32_t window_win32_close_window(WindowContext *context)
{
    assert(context != NULL);
    DestroyWindow(context->hwnd);
    return 0;
}

int32_t window_win32_poll_os_events(WindowContext *context)
{
    assert(context != NULL);

    MSG msg;
    LoggerInterface *logger = context->logger;
    SAFE_WHILE(
        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE),
        WINDOW_WIN32_MAX_OS_EVENTS_PER_FRAME,
        {
            LOG_WRN(logger, "Too many os events in one frame (%d), skipping events till next frame", WINDOW_WIN32_MAX_OS_EVENTS_PER_FRAME);
        })
    {
        if (msg.message == WM_QUIT)
        {
            WindowEvent window_event = {
                .type = WINDOW_EVENT_TYPE_QUIT,
            };
            window_win32_window_events_push(context, &window_event);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

int32_t window_win32_wait_for_os_events(WindowContext *context)
{
    return NOT_IMPLEMENTED(int32_t, context);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowContext *context = NULL;

    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
        context = (WindowContext *)pCreate->lpCreateParams;

        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)context);
    }
    else
    {
        context = (WindowContext *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (context == NULL)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
    case WM_PAINT:
        ValidateRect(hwnd, NULL);
        return 0;

    case WM_SIZE:
        TODO("Recreate shit!")
        return 0;

    case WM_KEYDOWN:
    case WM_KEYUP:
        WindowEventKey key = win32_key_to_window_event_key(wParam);
        WindowEvent window_event = {
            .type = WINDOW_EVENT_TYPE_KEY_PRESS,
            .data.key_press = {
                .is_pressed = uMsg == WM_KEYDOWN,
                .key = key,
            },
        };
        window_win32_window_events_push(context, &window_event);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
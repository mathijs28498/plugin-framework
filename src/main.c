#define UNICODE
#define _UNICODE
#define COBJMACROS

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <stdio.h>

typedef struct
{
    ID2D1Factory *d2dFactory;
    ID2D1HwndRenderTarget *renderTarget;
} RenderContext;

typedef struct
{
    ID2D1SolidColorBrush *primaryBrush;
} DrawResources;

typedef struct
{
    IDWriteFactory *dWriteFactory;
    IDWriteTextFormat *textFormat;
} TextResources;

typedef struct WindowState
{
    RenderContext renderContext;
    DrawResources drawResources;
    TextResources textResources;
} ApplicationState;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ApplicationState *GetApplicationState(HWND hwnd);

HRESULT CreateDeviceIndependentResources(ApplicationState *applicationState);
HRESULT CreateDeviceDependentResources(HWND hwnd, ApplicationState *applicationState);

void DiscardDeviceDependentResources(ApplicationState *application);
void ReleaseDeviceDependentResources(ApplicationState *applicationState);

void OnPaint(HWND hwnd, ApplicationState *applicationState);
void OnResize(HWND hwnd, ApplicationState *applicationState);

// TODO: Add error handling popup (look perplexity on how)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)pCmdLine;

    static ApplicationState applicationState = {0};

    // Initialize COM
    CoInitialize(NULL);

    HRESULT hr = CreateDeviceIndependentResources(&applicationState);
    if (FAILED(hr))
    {
        CoUninitialize();
        return 1;
    }

    const wchar_t CLASS_NAME[] = L"DirectWriteWindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        CLASS_NAME,
        L"DirectWrite Example",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        &applicationState);

    if (hwnd == NULL)
    {
        CoUninitialize();
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return 0;
}

ApplicationState *GetApplicationState(HWND hwnd)
{
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ApplicationState *applicationState = (ApplicationState *)ptr;
    return applicationState;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ApplicationState *applicationState;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT *createStruct = (CREATESTRUCT *)lParam;
        applicationState = (ApplicationState *)createStruct->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)applicationState);
    }
    else
    {
        applicationState = GetApplicationState(hwnd);
    }

    switch (uMsg)
    {
    case WM_PAINT:
        OnPaint(hwnd, applicationState);
        return 0;

    case WM_SIZE:
        OnResize(hwnd, applicationState);
        return 0;

    case WM_DESTROY:
        DiscardDeviceDependentResources(applicationState);
        ReleaseDeviceDependentResources(applicationState);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT CreateDeviceIndependentResources(ApplicationState *applicationState)
{
    HRESULT hr = S_OK;

    // Create D2D1 factory
    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &IID_ID2D1Factory,
        NULL,
        (void **)&applicationState->renderContext.d2dFactory);

    if (FAILED(hr))
        return hr;

    // Create DirectWrite factory
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        &IID_IDWriteFactory,
        (IUnknown **)&applicationState->textResources.dWriteFactory);

    if (FAILED(hr))
        return hr;

    {
        IDWriteTextFormat **textFormat = &applicationState->textResources.textFormat;

        hr = IDWriteFactory_CreateTextFormat(
            applicationState->textResources.dWriteFactory,
            L"Arial",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            15.0f,
            L"en-us",
            textFormat);

        if (SUCCEEDED(hr))
        {
            IDWriteTextFormat_SetTextAlignment(*textFormat, DWRITE_TEXT_ALIGNMENT_LEADING);
            IDWriteTextFormat_SetParagraphAlignment(*textFormat, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        }
    }

    return hr;
}

HRESULT CreateDeviceDependentResources(HWND hwnd, ApplicationState *applicationState)
{
    if (applicationState->renderContext.renderTarget != NULL)
    {
        return S_OK;
    }

    RECT rc;
    GetClientRect(hwnd, &rc);

    D2D1_SIZE_U size = {
        rc.right - rc.left,
        rc.bottom - rc.top};

    D2D1_RENDER_TARGET_PROPERTIES props = {
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        {DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN},
        0,
        0,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT};

    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = {
        hwnd,
        size,
        D2D1_PRESENT_OPTIONS_NONE};

    HRESULT hr = ID2D1Factory_CreateHwndRenderTarget(
        applicationState->renderContext.d2dFactory,
        &props,
        &hwndProps,
        &applicationState->renderContext.renderTarget);

    if (SUCCEEDED(hr))
    {
        D2D1_COLOR_F color = {1.0f, 1.0f, 1.0f, 1.0f};
        hr = ID2D1HwndRenderTarget_CreateSolidColorBrush(
            applicationState->renderContext.renderTarget,
            &color,
            NULL,
            &applicationState->drawResources.primaryBrush);
    }

    return hr;
}

void DiscardDeviceDependentResources(ApplicationState *applicationState)
{
    if (applicationState->drawResources.primaryBrush)
    {
        ID2D1SolidColorBrush_Release(applicationState->drawResources.primaryBrush);
        applicationState->drawResources.primaryBrush = NULL;
    }
    if (applicationState->renderContext.renderTarget)
    {
        ID2D1HwndRenderTarget_Release(applicationState->renderContext.renderTarget);
        applicationState->renderContext.renderTarget = NULL;
    }
}

void ReleaseDeviceDependentResources(ApplicationState *applicationState)
{
    if (applicationState->textResources.textFormat)
        IDWriteTextFormat_Release(applicationState->textResources.textFormat);
    if (applicationState->textResources.dWriteFactory)
        IDWriteFactory_Release(applicationState->textResources.dWriteFactory);
    if (applicationState->renderContext.d2dFactory)
        ID2D1Factory_Release(applicationState->renderContext.d2dFactory);
}

HRESULT CreateLineNumberString(int numLines, WCHAR *outBuffer, const size_t outBufferLen)
{
    // TODO: Add outBufferLen check and return an error;
    outBuffer[0] = L'\0';

    for (int i = 0; i < numLines; i++)
    {
        const size_t lineLength = 10;
        wchar_t line[lineLength];
        swprintf_s(line, lineLength, L"%d\n", i + 1);
        wcscat_s(outBuffer, outBufferLen, line);
    }

    return S_OK;
}

void OnPaint(HWND hwnd, ApplicationState *applicationState)
{
    HRESULT hr = CreateDeviceDependentResources(hwnd, applicationState);

    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);

        ID2D1HwndRenderTarget_BeginDraw(applicationState->renderContext.renderTarget);

        D2D1_COLOR_F clearColor = {.0f, .0f, .0f, 1.0f};
        ID2D1HwndRenderTarget_Clear(applicationState->renderContext.renderTarget, &clearColor);

        RECT rc;
        GetClientRect(hwnd, &rc);

        float paddingHorizontal = 10.0f;
        float paddingVertical = 10.0f;

        int numberOfLines = 30;
        // TODO: Look at this string length
        // TODO: Can make more faster by using proper lengths that are calculated
        const size_t lineNumberStringLen = 200;
        WCHAR lineNumberString[lineNumberStringLen];
        CreateLineNumberString(numberOfLines, lineNumberString, lineNumberStringLen);
        UINT32 lineNumberSize = (UINT32)wcslen(lineNumberString);

        IDWriteTextLayout *lineNumberTextLayout = NULL;
        IDWriteFactory_CreateTextLayout(
            applicationState->textResources.dWriteFactory,
            lineNumberString,
            lineNumberSize,
            applicationState->textResources.textFormat,
            (FLOAT)(rc.right - rc.left) - paddingHorizontal * 2.0f,
            (FLOAT)(rc.bottom - rc.top) - paddingVertical * 2.0f,
            &lineNumberTextLayout);

        D2D1_POINT_2F lineNumberTextLocation = {
            paddingHorizontal,
            paddingVertical};

        ID2D1HwndRenderTarget_DrawTextLayout(
            applicationState->renderContext.renderTarget,
            lineNumberTextLocation,
            lineNumberTextLayout,
            (ID2D1Brush *)applicationState->drawResources.primaryBrush,
            D2D1_DRAW_TEXT_OPTIONS_NONE);

        const WCHAR *userText = L"Hello Oyku!\nThis is working yes?";
        UINT32 userTextLength = (UINT32)wcslen(userText);

        float leftOffset = 30.0f;

        IDWriteTextLayout *userTextLayout = NULL;
        IDWriteFactory_CreateTextLayout(
            applicationState->textResources.dWriteFactory,
            userText,
            userTextLength,
            applicationState->textResources.textFormat,
            (FLOAT)(rc.right - rc.left) - paddingHorizontal * 2.0f - leftOffset,
            (FLOAT)(rc.bottom - rc.top) - paddingVertical * 2.0f,
            &userTextLayout);

        D2D1_POINT_2F userTextLocation = {
            paddingHorizontal + leftOffset,
            paddingVertical};

        ID2D1HwndRenderTarget_DrawTextLayout(
            applicationState->renderContext.renderTarget,
            userTextLocation,
            userTextLayout,
            (ID2D1Brush *)applicationState->drawResources.primaryBrush,
            D2D1_DRAW_TEXT_OPTIONS_NONE);

        // ID2D1HwndRenderTarget_DrawText(
        //     applicationState->renderContext.renderTarget,
        //     userText,
        //     userTextLength,
        //     applicationState->textResources.textFormat,
        //     &layoutRect,
        //     (ID2D1Brush *)applicationState->drawResources.primaryBrush,
        //     D2D1_DRAW_TEXT_OPTIONS_NONE,
        //     DWRITE_MEASURING_MODE_NATURAL);

        // const WCHAR *text_line_numbers = L"

        hr = ID2D1HwndRenderTarget_EndDraw(applicationState->renderContext.renderTarget, NULL, NULL);

        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceDependentResources(applicationState);
        }

        EndPaint(hwnd, &ps);
    }
}

void OnResize(HWND hwnd, ApplicationState *applicationState)
{
    if (applicationState->renderContext.renderTarget)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        D2D1_SIZE_U size = {
            rc.right - rc.left,
            rc.bottom - rc.top};
        ID2D1HwndRenderTarget_Resize(applicationState->renderContext.renderTarget, &size);
        InvalidateRect(hwnd, NULL, FALSE);
    }
}
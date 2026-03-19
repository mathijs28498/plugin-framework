#include <Windows.h>
#include <stdio.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)nCmdShow;
    (void)hPrevInstance;
    (void)lpCmdLine;

    printf("This works?\n");
    return 0;
}
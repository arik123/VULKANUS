#include "Window.h"


LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window* windowObject = (Window*)GetPropW(hwnd, L"this");
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        windowObject->windowResized = true;
        windowObject->resize = {
            LOWORD(lParam),//width
            HIWORD(lParam)//height
        };
        return 0;
    case WM_NCDESTROY:
        RemovePropW(hwnd, L"this");
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool Window::create() //todo dynamic class names
{
    WNDCLASSW wc;
    ZeroMemory(&wc, sizeof(wc));
    /* register window class */
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = WindowName;
    if (!RegisterClassW(&wc)) {
        std::cout << "class registration failed";
        std::cout << GetLastError();
        return true;
    }

    windowHandle = CreateWindowW(WindowName, L"Main Window", WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (windowHandle == NULL) {
        std::cout << "Window creation failed";
        return true;
    }
    SetPropW(windowHandle, L"this", this);
    ShowWindow(windowHandle, SW_SHOW);
    return false;
}

HWND Window::getHandle()
{
    return windowHandle;
}

/// <summary>
/// Gets current window size, if resetsWindowHasResized
/// </summary>
/// <returns>tuple(width,height)</returns>
std::tuple<uint32_t, uint32_t> Window::getSize()
{
    if (windowResized) {
        windowResized = false;
        return { resize.width, resize.height };
    }
    RECT window;
    if(!GetClientRect(windowHandle, &window)) throw "failed to get client rect";
    return { window.right, window.bottom };
    
}

inline bool Window::hasResized()
{
    return windowResized;
}

void Window::destroy()
{
    if (windowHandle) {
        DestroyWindow(windowHandle);
        windowHandle = nullptr;
    }
}
void Window::pollEvents()
{
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            printf("Recieved quit");
            close = true;
            return;
        }
        else if (msg.message == WM_CLOSE)
        {
            printf("Recieved close");
            close = true;
            return;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return;
}
bool Window::shouldClose()
{
    return close;
}
#pragma once
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#error "not implemented"
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#error "not implemented"
#elif defined(_WIN32)

#ifndef NOMINMAX

#endif // !NOMINMAX

#define NOMINMAX
#include <windows.h>

#endif
#include <iostream>
#include <tuple>

class Window
{
private:
	bool close = false;
	bool windowResized;
	HWND windowHandle = nullptr;
	LPCWSTR WindowName;
	struct {
		uint32_t width;
		uint32_t height;
	} resize;
	/*FUNCTIONS*/
private:
	static LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	Window() : WindowName(L"MainWindow") {};
	Window(LPCWSTR WindowName) : WindowName(WindowName){};
	bool create();
	HWND getHandle();
	std::tuple< uint32_t, uint32_t> getSize();
	inline bool hasResized();
	void destroy();
	void pollEvents();
	bool shouldClose();
};


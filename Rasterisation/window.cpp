﻿#include "window.h"

Window* window;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		exit(0);
		return 0;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		exit(0);
		return 0;
	}
	case WM_KEYDOWN:
	{
		window->keys[(unsigned int)wParam] = true;
		return 0;
	}
	case WM_KEYUP:
	{
		window->keys[(unsigned int)wParam] = false;
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
		window->mouseButtons[0] = true;
		return 0;
	}
	case WM_LBUTTONUP:
	{
		window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
		window->mouseButtons[0] = false;
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
		return 0;
	}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

void Window::Init(std::string window_name, int window_width, int window_height, int window_x, int window_y) {
	WNDCLASSEX wc;
	hInstance = GetModuleHandle(NULL);
	name = window_name;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	std::wstring wname = std::wstring(name.begin(), name.end());
	wc.lpszClassName = wname.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);

	width = window_width;
	height = window_height;
	style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, wname.c_str(), wname.c_str(), style, window_x, window_y, width, height, NULL, NULL, hInstance, this);
	memset(keys, 0, 256 * sizeof(char));
	window = this;

	// 
	ShowCursor(FALSE); // 隐藏鼠标
	GetClientRect(hwnd, &windowRect); // 获取窗口区域
	ClientToScreen(hwnd, (POINT*)&windowRect.left);
	ClientToScreen(hwnd, (POINT*)&windowRect.right);
	ClipCursor(&windowRect); // 限制鼠标在窗口内
}

void Window::updateMouse(int x, int y)
{
	mousex = x;
	mousey = y;
}

void Window::processMessages() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// 获取鼠标移动量
void Window::getMouseMovement(float& deltaX, float& deltaY) {
	POINT currentCursorPos;
	GetCursorPos(&currentCursorPos);

	if (firstMouse) {
		lastX = currentCursorPos.x;
		lastY = currentCursorPos.y;
		firstMouse = false;
	}

	// 计算鼠标移动量
	deltaX = static_cast<float>(lastX - currentCursorPos.x);	
	deltaY = static_cast<float>(lastY - currentCursorPos.y);


	HWND activeWindow = GetForegroundWindow();

	if (activeWindow == this->hwnd) {
		// 重置鼠标到窗口中心
		int centerX = (windowRect.left + windowRect.right) / 2;
		int centerY = (windowRect.top + windowRect.bottom) / 2;
		SetCursorPos(centerX, centerY);

		lastX = centerX;
		lastY = centerY;
	}
	else {
		// 如果不是活动窗口，不锁定鼠标
		lastX = currentCursorPos.x;
		lastY = currentCursorPos.y;
	}

}
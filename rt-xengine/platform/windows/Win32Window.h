#pragma once

#include <windows.h>
#include "system/Engine.h"
#include "platform/Window.h"

class Win32Window : public Window
{
	WNDCLASSEX m_wcex;
	HWND m_hWnd;

public:

	// Attach input before creating window
	Win32Window(Engine* engine);
	~Win32Window();

protected:
	// Window class styles. This is a combination of one or more Window Class Styles
	// Window class name. This class name is used by several functions to retrieve window information at run time.
	// Window background brush color. A brush handle representing the background color.
	// Window cursor
	// Window procedure associated to this window class. A pointer to a window procedure function. This function is used for message processing.
	// The application instance
	bool Register(
		UINT style,
		LPCSTR name,
		HBRUSH backgroundBrushColor,
		HCURSOR cursor,
		WNDPROC windowHandleFunction,
		HINSTANCE instance);

	bool Create(
		int32 xpos, 
		int32 ypox, 
		int32 width, 
		int32 height, 
		LPCSTR title, 
		LONG style);

public:

	static std::unique_ptr<Win32Window> CreateWin32Window(
		Engine* engine,
		const std::string& title = std::string("Win32 Window"),
		int32 xpos = 150,
		int32 ypox = 150,
		int32 width = 1920,
		int32 height = 1080,
		LONG cstyle = WS_OVERLAPPEDWINDOW,
		WNDPROC windowHandleFunction = WndProc,
		UINT style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
		LPCSTR name = TEXT("RTXENGINEWINDOWCLASS"),
		HBRUSH backgroundBrushColor = (HBRUSH)(COLOR_WINDOW + 1),
		LPCSTR cursorName = IDC_ARROW
	);

	HWND GetHWND() const { return m_hWnd; }
	HINSTANCE GetHInstance() const { return m_wcex.hInstance; }

	void RestrictMouseMovement() override;
	void ReleaseMouseMovement() override;

	void Show() override;

	bool StartRenderer(uint32 index) override;
	void HandleEvents(bool shouldHandleControllers) override;

	void SetTitle(const std::string& newTitle) override;

	void GenerateXInputControllerMessages();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

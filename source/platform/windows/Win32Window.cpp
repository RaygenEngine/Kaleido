#include "pch/pch.h"
#include "platform/windows/resources/WinResource.h"
#include "platform/windows/Win32Window.h"
#include "platform/windows/TranslateWin32VirtualKeys.h"
#include "system/Input.h"
#include "system/EngineEvents.h"
#include "editor/imgui/ImguiImpl.h"
#include <windowsx.h>

Win32Window::Win32Window()
	: m_wcex()
	, m_hWnd(nullptr)
{
}

Win32Window::~Win32Window()
{
	Hide();
	CloseWindow(m_hWnd);
	// Unregister window class, freeing the memory that was
	// previously allocated for this window.

	::UnregisterClass(m_wcex.lpszClassName, m_wcex.hInstance);
}

bool Win32Window::Register(UINT style, LPCSTR name, HBRUSH backgroundBrushColor, HCURSOR cursor,
	WNDPROC windowHandleFunction, HINSTANCE instance)
{
	// Setup window class attributes.
	ZeroMemory(&m_wcex, sizeof(m_wcex));

	m_wcex.cbSize = sizeof(m_wcex);
	m_wcex.style = style;
	m_wcex.lpszClassName = name;
	m_wcex.hbrBackground = backgroundBrushColor;
	m_wcex.hCursor = cursor;
	m_wcex.lpfnWndProc = windowHandleFunction;
	m_wcex.hInstance = instance;
	m_wcex.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));

	// Register window and ensure registration success.
	return static_cast<bool>(RegisterClassEx(&m_wcex));
}


bool Win32Window::Create(int32 xpos, int32 ypos, int32 width, int32 height, LPCSTR title, LONG style)
{
	// Setup window initialization attributes.
	CREATESTRUCT cs;
	ZeroMemory(&cs, sizeof(cs));

	cs.x = xpos;                         // Window X position
	cs.y = ypos;                         // Window Y position
	cs.cx = width;                       // Window width
	cs.cy = height;                      // Window height
	cs.hInstance = m_wcex.hInstance;     // Window instance.
	cs.lpszClass = m_wcex.lpszClassName; // Window class name
	cs.lpszName = title;                 // Window title
	cs.style = style;                    // Window style


	// Create the window.
	m_hWnd = ::CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy,
		cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);

	return m_hWnd;
}
char g_windowClass[100] = "";

Win32Window* Win32Window::CreateWin32Window(const std::string& title, int32 xpos, int32 ypox, int32 width, int32 height,
	LONG cstyle, WNDPROC windowHandleFunction, UINT style, LPCSTR name, HBRUSH backgroundBrushColor, LPCSTR cursorName)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	auto window = new Win32Window();
	strcpy_s(g_windowClass, (std::string(name) + std::to_string(rand())).c_str());
	if (!window->Register(style, g_windowClass, backgroundBrushColor, LoadCursor(nullptr, cursorName),
			windowHandleFunction, hInstance)) {
		LOG_ABORT("Failed to register application window! {}", GetLastError());
		return nullptr;
	}

	if (!window->Create(xpos, ypox, width, height, title.c_str(), cstyle)) {
		LOG_ABORT("Failed to create application window! {}", GetLastError());
		return nullptr;
	}

	return window;
}

void Win32Window::RestrictMouseMovement()
{
	// restrict mouse inside window
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	POINT ul = { rect.left, rect.top };
	POINT lr = { rect.right, rect.bottom };

	MapWindowPoints(m_hWnd, nullptr, &ul, 1);
	MapWindowPoints(m_hWnd, nullptr, &lr, 1);

	rect = { ul.x, ul.y, lr.x, lr.y };
	ClipCursor(&rect);
}

void Win32Window::ReleaseMouseMovement()
{
	ClipCursor(nullptr);
}

void Win32Window::Destroy()
{
	DestroyWindow(m_hWnd);
}

void Win32Window::Show()
{
	::ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	::UpdateWindow(m_hWnd);
}

void Win32Window::Hide()
{
	::ShowWindow(m_hWnd, SW_HIDE);
	::UpdateWindow(m_hWnd);
}

void Win32Window::DrawSplash()
{
}

void Win32Window::DrawLoading()
{
}

void Win32Window::GenerateXInputControllerMessages()
{
	const auto FindAvailableController = []() -> int {
		DWORD controllerIndex = 0;
		for (; controllerIndex < XUSER_MAX_COUNT; controllerIndex++) {
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));

			if (XInputGetState(controllerIndex, &state) == ERROR_SUCCESS) {
				LOG_INFO("Found active XInput controller, index: {}", controllerIndex);
				break;
			}
		}

		return controllerIndex;
	};


	// send changes directly to engine input, don't post messages its slow
	auto& input = *Engine::GetInput();

	// first active controller index
	static DWORD controllerIndex = FindAvailableController();

	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	XInputGetState(controllerIndex, &state);

	const auto CalculateStick = [](const glm::vec2& point, int32 deadZone) {
		Input::Stick stick{};

		const float magnitude = glm::length(point);
		stick.direction = glm::normalize(point);

		// clamp and normalize
		magnitude > deadZone ? stick.magnitude = (glm::min(magnitude, 32767.f) - deadZone) / (32767.f - deadZone)
							 : stick.magnitude = 0.0f;

		return stick;
	};

	Input::AnalogState analogState{};

	analogState.ls
		= CalculateStick({ state.Gamepad.sThumbLX, state.Gamepad.sThumbLY }, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	analogState.rs
		= CalculateStick({ state.Gamepad.sThumbRX, state.Gamepad.sThumbRY }, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

	const auto CalculateTrigger = [](BYTE magnitude, int32 deadZone) {
		return magnitude > deadZone ? (glm::min(static_cast<float>(magnitude), 255.f) - deadZone) / (255.f - deadZone)
									: 0.f;
	};

	analogState.lt = CalculateTrigger(state.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	analogState.rt = CalculateTrigger(state.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

	input.UpdateAnalogState(analogState);

	// check for keystrokes
	XINPUT_KEYSTROKE keystroke;
	ZeroMemory(&keystroke, sizeof(XINPUT_KEYSTROKE));
	XInputGetKeystroke(controllerIndex, {}, &keystroke);

	switch (keystroke.Flags) {
			// we only handle the keydown and keyup events, repeat is handled internally by the engine
		case XINPUT_KEYSTROKE_KEYDOWN: input.UpdateKeyPressed(TranslateXInputVirtualKeys(keystroke.VirtualKey)); break;

		case XINPUT_KEYSTROKE_KEYUP: input.UpdateKeyReleased(TranslateXInputVirtualKeys(keystroke.VirtualKey)); break;

		case XINPUT_KEYSTROKE_REPEAT:
		default:;
			// if no keystrokes, repeat is handled in engine - don't send repeat messages
	}
}

void Win32Window::HandleEvents(bool shouldHandleControllers)
{
	MSG msg;

	if (shouldHandleControllers) {
		GenerateXInputControllerMessages();
	}

	while (::PeekMessage(&msg, GetHWND(), 0, 0, PM_REMOVE) > 0) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void Win32Window::SetTitle(const std::string& newTitle)
{
	SetWindowText(GetHWND(), newTitle.c_str());
}

LRESULT CALLBACK Win32Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = NULL;

	Win32Window* window = Engine::GetMainWindow();

	if (!window) {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	if (Engine::IsEditorActive()) {
		if (ImguiImpl::WndProcHandler(hWnd, message, wParam, lParam)) {
			return true;
		}
	}

	auto& input = *Engine::GetInput();

	switch (message) {
		case WM_SETFOCUS:
			SetFocus(hWnd);
			window->m_focused = true;
			break;

		case WM_KILLFOCUS: window->m_focused = false; break;

		case WM_CLOSE:
		case WM_DESTROY:
			window->m_closed = true;
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			// escape loses focus - use to resize, close, etc.
			// also propagated in input
			if (wParam == VK_ESCAPE) {
				window->ReleaseMouseMovement();
			}

			// translate generic key press
			input.UpdateKeyPressed(TranslateWin32VirtualKeys(wParam));
			// map extended key press
			if (MapLeftRightKeys(wParam, lParam)) {
				input.UpdateKeyPressed(TranslateWin32VirtualKeys(wParam));
			}
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			// translate generic key up
			input.UpdateKeyReleased(TranslateWin32VirtualKeys(wParam));
			// map extended key up
			if (MapLeftRightKeys(wParam, lParam)) {
				input.UpdateKeyReleased(TranslateWin32VirtualKeys(wParam));
			}
			break;

		case WM_LBUTTONDOWN: input.UpdateKeyPressed(Key::LBUTTON); break;

		case WM_RBUTTONDOWN: input.UpdateKeyPressed(Key::RBUTTON); break;

		case WM_MBUTTONDOWN: input.UpdateKeyPressed(Key::MBUTTON); break;

		case WM_XBUTTONDOWN:
			input.UpdateKeyPressed(TranslateWin32VirtualKeys(MapExtraMouseButtons(GET_XBUTTON_WPARAM(wParam))));
			break;

		case WM_LBUTTONUP: input.UpdateKeyReleased(Key::LBUTTON); break;

		case WM_RBUTTONUP: input.UpdateKeyReleased(Key::RBUTTON); break;

		case WM_MBUTTONUP: input.UpdateKeyReleased(Key::MBUTTON); break;

		case WM_XBUTTONUP:
			input.UpdateKeyReleased(TranslateWin32VirtualKeys(MapExtraMouseButtons(GET_XBUTTON_WPARAM(wParam))));
			break;

		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
			window->RestrictMouseMovement();
			input.UpdateDoubleClick();
			break;

		case WM_MOUSEWHEEL: input.UpdateWheel(GET_WHEEL_DELTA_WPARAM(wParam)); break;

		case WM_MOUSEMOVE:
			// drag is considered any of the mouse buttons as modifier
			if (wParam & MK_LBUTTON || wParam & MK_MBUTTON || wParam & MK_RBUTTON || wParam & MK_XBUTTON1
				|| wParam & MK_XBUTTON2) {
				input.UpdateCursorDrag();
			}

			input.UpdateCursorPosition({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
			break;

		case WM_SIZE:
			// don't report ever 0 width/height to avoid numerical errors
			if (LOWORD(lParam) == 0 || HIWORD(lParam) == 0) {
				break;
			}

			window->m_width = LOWORD(lParam);
			window->m_height = HIWORD(lParam);

			Event::OnWindowResize.Broadcast(window->m_width, window->m_height);
			break;

		default: result = DefWindowProc(hWnd, message, wParam, lParam);
	}

	return result;
}

std::function<void()> Win32Window::RecreateWindow(WindowType*& currentWindow)
{
	Win32Window* prevWindow = dynamic_cast<Win32Window*>(currentWindow);
	CLOG_ABORT(!prevWindow, "Attempting to recreate window with wrong window type.");

	Win32Window* newWindow = new Win32Window();
	HWND oldHandle = prevWindow->GetHWND();

	WINDOWINFO inf{}; // Clone info
	inf.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(oldHandle, &inf);

	newWindow->m_wcex = prevWindow->m_wcex; // Clone class

	TCHAR str[256];
	GetWindowText(oldHandle, str, 256);


	WINDOWPLACEMENT placement{};
	placement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(oldHandle, &placement);


	// Setup window initialization attributes for new window.
	CREATESTRUCT cs;
	ZeroMemory(&cs, sizeof(cs));

	cs.x = inf.rcWindow.left;                       // Window X position
	cs.y = inf.rcWindow.top;                        // Window Y position
	cs.cx = inf.rcWindow.right - inf.rcWindow.left; // Window width
	cs.cy = inf.rcWindow.bottom - inf.rcWindow.top; // Window height
	cs.hInstance = newWindow->m_wcex.hInstance;     // Window instance.
	cs.lpszClass = newWindow->m_wcex.lpszClassName; // Window class name
	cs.lpszName = str;                              // Window title
	cs.style = inf.dwStyle;                         // Window style


	// Create the window.
	newWindow->m_hWnd = ::CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy,
		cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);

	// A bit hacky, we assign the proper window as "MainWindow" temporarily to circumvent how the event is handled.
	currentWindow = newWindow;
	newWindow->m_width = inf.rcClient.right - inf.rcClient.left;
	newWindow->m_height = inf.rcClient.bottom - inf.rcClient.top;
	Event::OnWindowResize.Broadcast(newWindow->m_width, newWindow->m_height);

	// "Show" the new window in background.
	::SetWindowPos(
		newWindow->m_hWnd, prevWindow->m_hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
	::UpdateWindow(newWindow->m_hWnd);
	SetWindowPlacement(newWindow->m_hWnd, &placement);

	// Do not hide the window here (because it will not draw). The old window should still be on top of the new one.

	// This lambda body runs at the end of main loop (where the renderer will have already presented one frame)
	return [&currentWindow, newWindow, prevWindow, placement]() {
		currentWindow = prevWindow;
		// Bring to front the new winow, before destroying the old one, this hides OS animations from playing on top of
		// the viewport.
		::SetWindowPos(prevWindow->m_hWnd, newWindow->m_hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		// Now we can hide and destroy our window. Our handler will know about the window as active because we set is as
		// "current window" temporarily.
		prevWindow->Hide();
		prevWindow->Destroy();
		delete prevWindow;

		// Reset engine's MainWindow to the newWindow;
		currentWindow = newWindow;
	};
}


std::function<std::function<void()>(WindowType*&)> Win32Window::GetRecreateWindowFunction()
{
	return { RecreateWindow };
}

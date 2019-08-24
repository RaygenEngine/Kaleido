#include "pch.h"

#include "Win32Window.h"
#include "renderer/Renderer.h"

#include <windowsx.h>

#include "TranslateWin32VirtualKeys.h"

namespace Platform
{
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	Win32Window::Win32Window(System::Engine* engineRef)
		: Window(engineRef),
		  m_wcex(),
		  m_hWnd(nullptr)
	{
	}

	Win32Window::~Win32Window()
	{
		// Unregister window class, freeing the memory that was
		// previously allocated for this window.

		::UnregisterClass(m_wcex.lpszClassName, m_wcex.hInstance);
	}

	bool Win32Window::Register(
		UINT style,
		LPCSTR name,
		HBRUSH backgroundBrushColor,
		HCURSOR cursor,
		HINSTANCE instance)
	{
		// Setup window class attributes.
		ZeroMemory(&m_wcex, sizeof(m_wcex));

		m_wcex.cbSize = sizeof(m_wcex);
		m_wcex.style = style;
		m_wcex.lpszClassName = name;
		m_wcex.hbrBackground = backgroundBrushColor;
		m_wcex.hCursor = cursor;
		m_wcex.lpfnWndProc = WndProc;
		m_wcex.hInstance = instance;

		// Register window and ensure registration success.
		return static_cast<bool>(RegisterClassEx(&m_wcex));
	}


	bool Win32Window::Create(
		int32 xpos,
		int32 ypox,
		int32 width,
		int32 height,
		LPCSTR title,
		LONG style)
	{
		// Setup window initialization attributes.
		CREATESTRUCT cs;
		ZeroMemory(&cs, sizeof(cs));

		cs.x = xpos;	                       // Window X position
		cs.y = ypox;	                       // Window Y position
		cs.cx = width;	                       // Window width
		cs.cy = height;	                       // Window height
		cs.hInstance = m_wcex.hInstance;       // Window instance.
		cs.lpszClass = m_wcex.lpszClassName;   // Window class name
		cs.lpszName = title;				   // Window title
		cs.style = style;					   // Window style

		// Create the window.
		m_hWnd = ::CreateWindowEx(
			cs.dwExStyle,
			cs.lpszClass,
			cs.lpszName,
			cs.style,
			cs.x,
			cs.y,
			cs.cx,
			cs.cy,
			cs.hwndParent,
			cs.hMenu,
			cs.hInstance,
			cs.lpCreateParams);

		return m_hWnd;
	}

	std::unique_ptr<Win32Window> Win32Window::CreateWin32Window(
		System::Engine* engineRef,
		const std::string& title,
		int32 xpos,
		int32 ypox,
		int32 width,
		int32 height,
		LONG cstyle,
		UINT style,
		LPCSTR name,
		HBRUSH backgroundBrushColor,
		LPCSTR cursorName)
	{
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		// attach engine to window
		auto window = std::make_unique<Win32Window>(engineRef);
		
		if (!window->Register(style, name, backgroundBrushColor,
							  LoadCursor(NULL, cursorName), hInstance))
		{
			RT_XENGINE_LOG_FATAL("Failed to register application window!");
			return std::unique_ptr<Win32Window>{};
		}


		if (!window->Create(xpos, ypox, width, height, title.c_str(), cstyle))
		{
			RT_XENGINE_LOG_FATAL("Failed to create application window!");
			return std::unique_ptr<Win32Window>{};
		}

		// pass self pointer inside the hwnd userdata (minor hack for callbacks)
		SetWindowLongPtr(window->GetHWND(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window.get()));

		return std::move(window);
	}

	void Win32Window::RestrictMouseMovement()
	{
		// restrict mouse inside window
		RECT rect;
		GetClientRect(m_hWnd, &rect);

		POINT ul = { rect.left,rect.top };
		POINT lr = { rect.right, rect.bottom };

		MapWindowPoints(m_hWnd, nullptr, &ul, 1);
		MapWindowPoints(m_hWnd, nullptr, &lr, 1);

		rect = { ul.x, ul.y, lr.x, lr.y };
		ClipCursor(&rect);
	}

	void Win32Window::ReleaseMouseMovement()
	{
		ClipCursor(NULL);
	}

	void Win32Window::Show()
	{
		::ShowWindow(m_hWnd, SW_SHOWDEFAULT);
		::UpdateWindow(m_hWnd);
	}

	void Win32Window::GenerateXInputControllerMessages()
	{
		const auto FindAvailableController = []() -> int
		{
			DWORD controllerIndex = 0;
			for (; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
			{
				XINPUT_STATE state;
				ZeroMemory(&state, sizeof(XINPUT_STATE));

				if (XInputGetState(controllerIndex, &state) == ERROR_SUCCESS)
				{
					RT_XENGINE_LOG_INFO("Found active XInput controller, index: {}", controllerIndex);
					break;
				}
			}

			return controllerIndex;
		};


		// send changes directly to engine input, don't post messages its slow
		auto& input = m_engineRef->GetInput();

		// first active controller index
		static DWORD controllerIndex = FindAvailableController();

		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		XInputGetState(controllerIndex, &state);

		const auto CalculateThumb = [](const glm::vec2& point, int32 deadZone)
		{
			Input::Thumb thumb{};

			const float magnitude = glm::length(point);
			thumb.direction = glm::normalize(point);

			// clamp and normalize
			magnitude > deadZone ? thumb.magnitude = (glm::min(magnitude, 32767.f) - deadZone) / (32767 - deadZone) : thumb.magnitude = 0.0f;

			return thumb;
		};

		Input::AnalogState analogState{};

		analogState.thumbL = CalculateThumb({ state.Gamepad.sThumbLX , state.Gamepad.sThumbLY }, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		analogState.thumbR = CalculateThumb({ state.Gamepad.sThumbRX , state.Gamepad.sThumbRY }, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

		const auto CalculateTrigger = [](BYTE magnitude, int32 deadZone)
		{
			return magnitude > deadZone ? (glm::min(static_cast<float>(magnitude), 255.f) - deadZone) / (255.f - deadZone) : 0.f;
		};

		analogState.triggerL = CalculateTrigger(state.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		analogState.triggerR = CalculateTrigger(state.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

		input.UpdateAnalogState(analogState);

		// check for keystrokes
		XINPUT_KEYSTROKE keystroke;
		ZeroMemory(&keystroke, sizeof(XINPUT_KEYSTROKE));
		XInputGetKeystroke(controllerIndex, {}, &keystroke);

		UINT msg{};

		switch (keystroke.Flags)
		{
			// we only handle the keydown and keyup events, repeat is handled internally by the engine 
		case XINPUT_KEYSTROKE_KEYDOWN:
			input.UpdateKeyPressed(TranslateXInputVirtualKeys(keystroke.VirtualKey));
			break;

		case XINPUT_KEYSTROKE_KEYUP:
			input.UpdateKeyReleased(TranslateXInputVirtualKeys(keystroke.VirtualKey));
			break;

		case XINPUT_KEYSTROKE_REPEAT:
		default:;
			// if no keystrokes, repeat is handled in engine - don't send repeat messages
		}
	}

	void Win32Window::HandleEvents(bool shouldHandleControllers)
	{
		MSG msg;

		if (shouldHandleControllers) 
		{
			GenerateXInputControllerMessages();
		}

		while (::PeekMessage(&msg, GetHWND(), 0, 0, PM_REMOVE) > 0)
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	void Win32Window::SetTitle(const std::string& newTitle)
	{
		SetWindowText(GetHWND(), newTitle.c_str());
	}

	bool Win32Window::StartRenderer(System::RendererRegistrationIndex index) 
	{
		return m_engineRef->SwitchRenderer(index) &&
			m_engineRef->GetRenderer()->InitRendering(GetHWND(), GetHInstance()) &&
			m_engineRef->GetRenderer()->InitScene(GetWidth(), GetHeight());
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = NULL;

		auto* window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if(!window) return DefWindowProc(hWnd, message, wParam, lParam);

		auto& input = window->m_engineRef->GetInput();

		switch (message)
		{
		case WM_SETFOCUS:
			SetFocus(hWnd);
			window->m_focused = true;
			break;

		case WM_KILLFOCUS:
			window->m_focused = false;
			break;

		case WM_CLOSE:
		case WM_DESTROY:
			window->m_closed = true;
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			// escape loses focus - use to resize, close, etc.
			// also propagated in input
			if (wParam == VK_ESCAPE)
				window->ReleaseMouseMovement();

			// translate generic key press 
			input.UpdateKeyPressed(TranslateWin32VirtualKeys(wParam));
			// map extended key press
			if(MapLeftRightKeys(wParam, lParam))
				input.UpdateKeyPressed(TranslateWin32VirtualKeys(wParam));
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			// translate generic key up 
			input.UpdateKeyReleased(TranslateWin32VirtualKeys(wParam));
			// map extended key up
			if (MapLeftRightKeys(wParam, lParam))
				input.UpdateKeyReleased(TranslateWin32VirtualKeys(wParam));
			break;

		case WM_LBUTTONDOWN:
			input.UpdateKeyPressed(XVK_LBUTTON);
			break;

		case WM_RBUTTONDOWN:
			input.UpdateKeyPressed(XVK_RBUTTON);
			break;

		case WM_MBUTTONDOWN:
			input.UpdateKeyPressed(XVK_MBUTTON);
			break;

		case WM_XBUTTONDOWN:
			input.UpdateKeyPressed(TranslateWin32VirtualKeys(MapExtraMouseButtons(GET_XBUTTON_WPARAM(wParam))));
			break;

		case WM_LBUTTONUP:
			input.UpdateKeyReleased(XVK_LBUTTON);
			break;

		case WM_RBUTTONUP:
			input.UpdateKeyReleased(XVK_RBUTTON);
			break;

		case WM_MBUTTONUP:
			input.UpdateKeyReleased(XVK_MBUTTON);
			break;

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

		case WM_MOUSEWHEEL:
			input.UpdateWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		case WM_MOUSEMOVE:
			// drag is considered any of the mouse buttons as modifier
			if (wParam & MK_LBUTTON ||
				wParam & MK_MBUTTON ||
				wParam & MK_RBUTTON ||
				wParam & MK_XBUTTON1 ||
				wParam & MK_XBUTTON2)
				input.UpdateCursorDrag();

			input.UpdateCursorPosition({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
			break;

		case WM_SIZE:
			// don't report ever 0 width/height to avoid numerical errors
			if (LOWORD(lParam) == 0 || HIWORD(lParam) == 0)
				break;

			window->m_width = LOWORD(lParam);
			window->m_height = HIWORD(lParam);

			window->m_engineRef->GetWorld()->WindowResize(LOWORD(lParam), HIWORD(lParam));
			window->m_engineRef->GetRenderer()->WindowResize(LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			result = DefWindowProc(hWnd, message, wParam, lParam);
		}

		return result;
	}

}

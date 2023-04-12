#include "LSEFramework.h"

import Platform.Win32Window;
import Util.StdUtils;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    LS::Win32::Win32Window* pWindow = nullptr;
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lparam);
        pWindow = reinterpret_cast<LS::Win32::Win32Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));

        if (pWindow)
        {
            return pWindow->HandleWinMessage(hWnd, message, wparam, lparam);
        }
    }
    else
    {
        pWindow = reinterpret_cast<LS::Win32::Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pWindow)
        {
            return pWindow->HandleWinMessage(hWnd, message, wparam, lparam);
        }
    };
    return DefWindowProc(hWnd, message, wparam, lparam);
}

namespace LS::Win32
{
    void Win32Window::Show()
    {
        ShowWindow(reinterpret_cast<HWND>(m_winHandle), SW_SHOW);
		m_bIsRunning = true;
    }

    void Win32Window::Close()
    {
		PostQuitMessage(0);
		m_bIsRunning = false;;
    }

    void Win32Window::PollEvent()
    {
		if (PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&m_msg);
			DispatchMessage(&m_msg);
		}
    }

    LS::LSWindowHandle Win32Window::GetHandleToWindow() const
    {
        return m_winHandle;
    }

    LRESULT Win32Window::HandleWinMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
		if (!m_bIsRunning)
			return DefWindowProc(hwnd, message, wparam, lparam);

		switch (message)
		{
		case (WM_NCCREATE):
		{
		}
		case (WM_PAINT):
		{
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		}
		case (WM_CLOSE):
		{
			PostQuitMessage(0);
			m_bIsRunning = false;
			if (m_onWindowEvent)
				m_onWindowEvent(LS::LS_WINDOW_EVENT::CLOSE_WINDOW);
			break;
		}
		case(WM_DESTROY):
		{
			PostQuitMessage(0);
			m_bIsRunning = false;
			if (m_onWindowEvent)
				m_onWindowEvent(LS::LS_WINDOW_EVENT::CLOSE_WINDOW);
			break;
		}
		case(WM_CHAR):
		{
			break;
		}
		case(WM_KEYDOWN):
		{
			OnKeyPress(wparam);
			break;
		}
		case (WM_KEYUP):
		{
			OnKeyRelease(wparam);
			break;
		}
		case(WM_LBUTTONDOWN):
		case(WM_MBUTTONDOWN):
		case(WM_RBUTTONDOWN):
		{
			int x;
			int y;
			GetScreenCoordinates(lparam, x, y);
			OnMouseClick(message, x, y);
			break;
		}
		case(WM_LBUTTONUP):
		case(WM_MBUTTONUP):
		case(WM_RBUTTONUP):
		{
			int x;
			int y;
			GetScreenCoordinates(lparam, x, y);
			OnMouseRelease(message, x, y);
			break;
		}
		case(WM_MOUSEMOVE):
		{
			int x;
			int y;
			GetScreenCoordinates(lparam, x, y);
			OnMouseMove(x, y);
			break;
		}
		case(WM_MOUSELEAVE):
		{
			if (m_onWindowEvent)
			{
				m_onWindowEvent(LS::LS_WINDOW_EVENT::LEAVE_WINDOW);
			}
			break;
		}
		case(WM_MOUSEHOVER):
		{
			//TODO: Requires us to do some tracking to get this event. Look up MSDN for info
			if (m_onWindowEvent)
			{
				m_onWindowEvent(LS::LS_WINDOW_EVENT::ENTER_WINDOW);
			}
			break;
		}
		case(WM_MOUSEWHEEL):
		{
			auto zDelta = GET_WHEEL_DELTA_WPARAM(wparam);
			OnMouseWheelScroll(zDelta);
			break;
		}
		case(WM_SIZE):
		{
			m_width = GET_X_LPARAM(lparam);
			m_height = GET_Y_LPARAM(lparam);
			if (wparam == SIZE_MAXIMIZED)
			{
				ResizeWindow(m_width, m_height);
				if (m_onWindowEvent)
					m_onWindowEvent(LS::LS_WINDOW_EVENT::MAXIMIZED_WINDOW);
			}
			else if (wparam == SIZE_MINIMIZED)
			{
				m_bIsResizing = true;
				if (m_onWindowEvent)
					m_onWindowEvent(LS::LS_WINDOW_EVENT::MINIMIZED_WINDOW);
			}
			else if (wparam == SIZE_RESTORED)
			{
				m_bIsResizing = false;
				ResizeWindow(m_width, m_height);
				if (m_onWindowEvent)
					m_onWindowEvent(LS::LS_WINDOW_EVENT::RESTORED_WINDOW);
			}
			break;
		}
		case (WM_SIZING):
		{
			m_bIsResizing = true;
			break;
		}
		case (WM_ENTERSIZEMOVE):
		{
			m_bIsResizing = true;
			break;
		}
		case (WM_EXITSIZEMOVE):
		{
			m_bIsResizing = false;
			if (!m_bIsResizing)
			{
				/*TRACE("Window exit size move!\n");
				if (m_prevHeight != m_height || m_prevWidth != m_width)
				{
					m_prevHeight = m_height;
					m_prevWidth = m_width;
					ResizeWindow(m_width, m_height);
				}*/
			}
			break;
		}
		case (WM_KILLFOCUS):
		{
			if (m_onWindowEvent)
			{
				m_onWindowEvent(LS::LS_WINDOW_EVENT::LOST_FOCUS);
			}
			break;
		}
		case (WM_SETFOCUS):
		{
			if (m_onWindowEvent)
			{
				m_onWindowEvent(LS::LS_WINDOW_EVENT::GAIN_FOCUS);
			}
			break;
		}
		default:
		{
			return DefWindowProc(hwnd, message, wparam, lparam);
		}
		}

		return 0;
    }

    void Win32Window::Initialize(uint32_t width, uint32_t height, std::wstring_view title) 
    {
        WNDCLASSEX wc{};
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = WndProc;
        wc.lpszClassName = title.data();
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);

        if (!RegisterClassEx(&wc))
        {
            // Error Handle 
            throw std::runtime_error("Failed to create window");
        }

        auto handle = CreateWindowEx(0,
            title.data(),
            title.data(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            width, height,
            NULL,
            NULL,
            reinterpret_cast<HINSTANCE>(m_hInstance),
            this);

        if (!handle)
        {
            auto result = HRESULT_FROM_WIN32(GetLastError());
            auto format = std::format("Failed to create Window: {}", result);
            std::cout << format;
            throw std::runtime_error(format.c_str());
        }

        m_winHandle = reinterpret_cast<LSWindowHandle>(handle);
    }

	void Win32Window::OnKeyPress(WPARAM wp)
	{
		if (!m_onKeyboardInput)
			return;
		auto input = ToInputKey(wp);
		int16_t mods = 0;
		using namespace LS::Utils;
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			mods |= ToIntegral(LS::LS_INPUT_MODS::SHIFT);
		}

		if (GetAsyncKeyState(VK_MENU) & 0x08000)
		{
			mods |= ToIntegral(LS::LS_INPUT_MODS::ALT);
		}

		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			mods |= ToIntegral(LS::LS_INPUT_MODS::CTRL);
		}

		auto action = LS::LS_INPUT_ACTION::PRESS;

		m_onKeyboardInput(ToIntegral(input), mods, ToIntegral(action));
	}

	void Win32Window::OnKeyRelease(WPARAM wp)
	{
		if (!m_onKeyboardInput)
			return;

		auto input = ToInputKey(wp);
		int16_t mods = 0;
		using namespace LS::Utils;
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			mods |= ToIntegral(LS::LS_INPUT_MODS::SHIFT);
		}

		if (GetAsyncKeyState(VK_MENU) & 0x8000)
		{
			mods |= ToIntegral(LS::LS_INPUT_MODS::ALT);
		}

		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			mods |= ToIntegral(LS::LS_INPUT_MODS::CTRL);
		}

		auto action = LS::LS_INPUT_ACTION::RELEASE;

		m_onKeyboardInput(ToIntegral(input), mods, ToIntegral(action));
	}
	
	inline void Win32Window::GetScreenCoordinates(LPARAM lp, int& x, int& y)
	{
		x = GET_X_LPARAM(lp);
		y = GET_Y_LPARAM(lp);
	}

	void Win32Window::OnMouseMove(int x, int y)
	{
		TrackMouse(reinterpret_cast<HWND>(m_winHandle));
		if (!m_onCursorMove)
			return;

		m_onCursorMove(x, y);
	}

	void Win32Window::OnMouseClick(UINT msg, int x, int y)
	{
		if (!m_onMouseInput)
			return;
		using M = LS::LS_INPUT_MOUSE;
		using MOD = LS::LS_INPUT_MODS;
		using A = LS::LS_INPUT_ACTION;
		using namespace LS::Utils;
		if (msg == WM_LBUTTONDOWN)
		{
			m_onMouseInput(ToIntegral(M::LMB), ToIntegral(MOD::NONE), ToIntegral(A::PRESS),
				x, y);
		}
		if (msg == WM_MBUTTONDOWN)
		{
			m_onMouseInput(ToIntegral(M::MMB), ToIntegral(MOD::NONE), ToIntegral(A::PRESS),
				x, y);
		}
		if (msg == WM_RBUTTONDOWN)
		{
			m_onMouseInput(ToIntegral(M::RMB), ToIntegral(MOD::NONE), ToIntegral(A::PRESS),
				x, y);
		}
	}
	
	void Win32Window::OnMouseRelease(UINT msg, int x, int y)
	{
		if (!m_onMouseInput)
			return;
		using M = LS::LS_INPUT_MOUSE;
		using MOD = LS::LS_INPUT_MODS;
		using A = LS::LS_INPUT_ACTION;
		using namespace LS::Utils;
		if (msg == WM_LBUTTONDOWN)
		{
			m_onMouseInput(ToIntegral(M::LMB), ToIntegral(MOD::NONE), ToIntegral(A::RELEASE),
				x, y);
		}
		if (msg == WM_MBUTTONDOWN)
		{
			m_onMouseInput(ToIntegral(M::MMB), ToIntegral(MOD::NONE), ToIntegral(A::RELEASE),
				x, y);
		}
		if (msg == WM_RBUTTONDOWN)
		{
			m_onMouseInput(ToIntegral(M::RMB), ToIntegral(MOD::NONE), ToIntegral(A::RELEASE),
				x, y);
		}
	}
	
	void Win32Window::OnMouseWheelScroll(short zDelta)
	{
		if (!m_onMouseWheelScroll)
			return;
		double scale = 1.0;

		double diff = zDelta / scale;

		m_onMouseWheelScroll(diff);
	}
	
	int Win32Window::ToWindowsKey(LS::LS_INPUT_KEY key)
	{
		using enum LS::LS_INPUT_KEY;
		switch (key)
		{
		case A:			return 0x41;
		case B:			return 0x42;
		case C:			return 0x43;
		case D:			return 0x44;
		case E:			return 0x45;
		case F:			return 0x46;
		case G:			return 0x47;
		case H:			return 0x48;
		case I:			return 0x49;
		case J:			return 0x4A;
		case K:			return 0x4B;
		case L:			return 0x4C;
		case M:			return 0x4D;
		case N:			return 0x4E;
		case O:			return 0x4F;
		case P:			return 0x50;
		case Q:			return 0x51;
		case R:			return 0x52;
		case S:			return 0x53;
		case T:			return 0x54;
		case U:			return 0x55;
		case V:			return 0x56;
		case W:			return 0x57;
		case X:			return 0x58;
		case Y:			return 0x59;
		case Z:			return 0x5A;
			// Numbers //
		case D0:			return 0x30;
		case D1:			return 0x31;
		case D2:			return 0x32;
		case D3:			return 0x33;
		case D4:			return 0x34;
		case D5:			return 0x35;
		case D6:			return 0x36;
		case D7:			return 0x37;
		case D8:			return 0x38;
		case D9:			return 0x39;
		case NUM_0:		return VK_NUMPAD0;
		case NUM_1:		return VK_NUMPAD1;
		case NUM_2:		return VK_NUMPAD2;
		case NUM_3:		return VK_NUMPAD3;
		case NUM_4:		return VK_NUMPAD4;
		case NUM_5:		return VK_NUMPAD5;
		case NUM_6:		return VK_NUMPAD6;
		case NUM_7:		return VK_NUMPAD7;
		case NUM_8:		return VK_NUMPAD8;
		case NUM_9:		return VK_NUMPAD9;
			// Special Keys //
		case AR_DOWN:		return VK_DOWN;
		case AR_UP:		return VK_UP;
		case AR_RIGHT:		return VK_RIGHT;
		case AR_LEFT:		return VK_LEFT;
		case BACK_TICK:	return VK_OEM_3;
		case MINUS:		return VK_OEM_MINUS;
		case EQUALS:		return VK_OEM_PLUS;
		case L_CURLY:		return VK_OEM_4;
		case R_CURLY:		return VK_OEM_6;
		case F_SLASH:		return VK_OEM_2;
		case B_SLASH:		return VK_OEM_5;
		case SINGLE_QUOTE:	return VK_OEM_7;
		case COMMA:		return VK_OEM_1;
		case PERIOD:		return VK_OEM_PERIOD;
		case APOSTROPHE:	return VK_OEM_7;
		case SPACEBAR:		return VK_SPACE;
		case L_SHIFT:		return VK_LSHIFT;
		case R_SHIFT:		return VK_RSHIFT;
		case L_ALT:		return VK_LMENU;
		case R_ALT:		return VK_RMENU;
		case L_CTRL:		return VK_LCONTROL;
		case R_CTRL:		return VK_RCONTROL;
			// Function Keys //
		case F1:			return VK_F1;
		case F2:			return VK_F2;
		case F3:			return VK_F3;
		case F4:			return VK_F4;
		case F5:			return VK_F5;
		case F6:			return VK_F6;
		case F7:			return VK_F7;
		case F8:			return VK_F8;
		case F9:			return VK_F9;
		case F10:			return VK_F10;
		case F11:			return VK_F11;
		case F12:			return VK_F12;
		}
		return 0x00;
	}
	
	LS::LS_INPUT_KEY Win32Window::ToInputKey(WPARAM wparam)
	{
		using enum LS::LS_INPUT_KEY;
		switch (wparam)
		{
			// Letters
		case 0x41:			return A;
		case 0x42:			return B;
		case 0x43:			return C;
		case 0x44:			return D;
		case 0x45:			return E;
		case 0x46:			return F;
		case 0x47:			return G;
		case 0x48:			return H;
		case 0x49:			return I;
		case 0x4A:			return J;
		case 0x4B:			return K;
		case 0x4C:			return L;
		case 0x4D:			return M;
		case 0x4E:			return N;
		case 0x4F:			return O;
		case 0x50:			return P;
		case 0x51:			return Q;
		case 0x52:			return R;
		case 0x53:			return S;
		case 0x54:			return T;
		case 0x55:			return U;
		case 0x56:			return V;
		case 0x57:			return W;
		case 0x58:			return X;
		case 0x59:			return Y;
		case 0x5A:			return Z;
			// Numbers
		case 0x30:			return D0;
		case 0x31:			return D1;
		case 0x32:			return D2;
		case 0x33:			return D3;
		case 0x34:			return D4;
		case 0x35:			return D5;
		case 0x36:			return D6;
		case 0x37:			return D7;
		case 0x38:			return D8;
		case 0x39:			return D9;
			// Arrow keys
		case VK_LEFT:		return AR_LEFT;
		case VK_RIGHT:		return AR_RIGHT;
		case VK_DOWN:		return AR_DOWN;
		case VK_UP:			return AR_UP;
			// Functions Keys
		case VK_F1:			return F1;
		case VK_F2:			return F2;
		case VK_F3:			return F3;
		case VK_F4:			return F4;
		case VK_F5:			return F5;
		case VK_F6:			return F6;
		case VK_F7:			return F7;
		case VK_F8:			return F8;
		case VK_F9:			return F9;
		case VK_F10:		return F10;
		case VK_F11:		return F11;
		case VK_F12:		return F12;
			// Special Keys
		case VK_LSHIFT:		return L_SHIFT;
		case VK_RSHIFT:		return R_SHIFT;
		case VK_LCONTROL:	return L_CTRL;
		case VK_RCONTROL:	return R_CTRL;
		case VK_LMENU:		return L_ALT;
		case VK_RMENU:		return R_ALT;
		case VK_ESCAPE:		return ESCAPE;
		default: return NONE;
		}
	}
	
	void Win32Window::TrackMouse(HWND hwnd)
	{
		TRACKMOUSEEVENT tme{};
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1;
		tme.hwndTrack = hwnd;
		TrackMouseEvent(&tme);
	}

	void Win32Window::ResizeWindow(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		if (m_onWindowEvent)
			m_onWindowEvent(LS::LS_WINDOW_EVENT::RESIZE_WINDOW);
	}
}
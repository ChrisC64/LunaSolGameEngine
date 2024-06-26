module;
#include <cstdint>
#include <string_view>
#include <format>
#include <functional>
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
export module Platform.Win32Window;

import Engine.LSWindow;
import Engine.Input;

export namespace LS::Win32
{
    using WndProcHandler = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

    LS::Input::KEYBOARD ToInputKey(WPARAM wparam)
    {
        using enum LS::Input::KEYBOARD;
        switch (wparam)
        {
            // Letters
        case 0x41:          return A;
        case 0x42:          return B;
        case 0x43:          return C;
        case 0x44:          return D;
        case 0x45:          return E;
        case 0x46:          return F;
        case 0x47:          return G;
        case 0x48:          return H;
        case 0x49:          return I;
        case 0x4A:          return J;
        case 0x4B:          return K;
        case 0x4C:          return L;
        case 0x4D:          return M;
        case 0x4E:          return N;
        case 0x4F:          return O;
        case 0x50:          return P;
        case 0x51:          return Q;
        case 0x52:          return R;
        case 0x53:          return S;
        case 0x54:          return T;
        case 0x55:          return U;
        case 0x56:          return V;
        case 0x57:          return W;
        case 0x58:          return X;
        case 0x59:          return Y;
        case 0x5A:          return Z;
            // Numbers
        case 0x30:          return D0;
        case 0x31:          return D1;
        case 0x32:          return D2;
        case 0x33:          return D3;
        case 0x34:          return D4;
        case 0x35:          return D5;
        case 0x36:          return D6;
        case 0x37:          return D7;
        case 0x38:          return D8;
        case 0x39:          return D9;
            // Arrow keys
        case VK_LEFT:       return AR_LEFT;
        case VK_RIGHT:      return AR_RIGHT;
        case VK_DOWN:       return AR_DOWN;
        case VK_UP:         return AR_UP;
            // Functions Keys
        case VK_F1:         return F1;
        case VK_F2:         return F2;
        case VK_F3:         return F3;
        case VK_F4:         return F4;
        case VK_F5:         return F5;
        case VK_F6:         return F6;
        case VK_F7:         return F7;
        case VK_F8:         return F8;
        case VK_F9:         return F9;
        case VK_F10:        return F10;
        case VK_F11:        return F11;
        case VK_F12:        return F12;
            // Special Keys
        case VK_LSHIFT:     return L_SHIFT;
        case VK_RSHIFT:     return R_SHIFT;
        case VK_LCONTROL:   return L_CTRL;
        case VK_RCONTROL:   return R_CTRL;
        case VK_LMENU:      return L_ALT;
        case VK_RMENU:      return R_ALT;
        case VK_ESCAPE:     return ESCAPE;
        default:            return NONE;
        }
    }

    int ToWindowsKey(LS::Input::KEYBOARD key)
    {
        using enum LS::Input::KEYBOARD;
        switch (key)
        {
        case A:          return 0x41;
        case B:          return 0x42;
        case C:          return 0x43;
        case D:          return 0x44;
        case E:          return 0x45;
        case F:          return 0x46;
        case G:          return 0x47;
        case H:          return 0x48;
        case I:          return 0x49;
        case J:          return 0x4A;
        case K:          return 0x4B;
        case L:          return 0x4C;
        case M:          return 0x4D;
        case N:          return 0x4E;
        case O:          return 0x4F;
        case P:          return 0x50;
        case Q:          return 0x51;
        case R:          return 0x52;
        case S:          return 0x53;
        case T:          return 0x54;
        case U:          return 0x55;
        case V:          return 0x56;
        case W:          return 0x57;
        case X:          return 0x58;
        case Y:          return 0x59;
        case Z:          return 0x5A;
            // Numbers //
        case D0:         return 0x30;
        case D1:         return 0x31;
        case D2:         return 0x32;
        case D3:         return 0x33;
        case D4:         return 0x34;
        case D5:         return 0x35;
        case D6:         return 0x36;
        case D7:         return 0x37;
        case D8:         return 0x38;
        case D9:         return 0x39;
        case NUM_0:      return VK_NUMPAD0;
        case NUM_1:      return VK_NUMPAD1;
        case NUM_2:      return VK_NUMPAD2;
        case NUM_3:      return VK_NUMPAD3;
        case NUM_4:      return VK_NUMPAD4;
        case NUM_5:      return VK_NUMPAD5;
        case NUM_6:      return VK_NUMPAD6;
        case NUM_7:      return VK_NUMPAD7;
        case NUM_8:      return VK_NUMPAD8;
        case NUM_9:      return VK_NUMPAD9;
            // Special Keys //
        case AR_DOWN:    return VK_DOWN;
        case AR_UP:      return VK_UP;
        case AR_RIGHT:   return VK_RIGHT;
        case AR_LEFT:    return VK_LEFT;
        case BACK_TICK:  return VK_OEM_3;
        case MINUS:      return VK_OEM_MINUS;
        case EQUALS:     return VK_OEM_PLUS;
        case L_CURLY:    return VK_OEM_4;
        case R_CURLY:    return VK_OEM_6;
        case F_SLASH:    return VK_OEM_2;
        case B_SLASH:    return VK_OEM_5;
        case SINGLE_QUOTE:return VK_OEM_7;
        case COMMA:      return VK_OEM_1;
        case PERIOD:     return VK_OEM_PERIOD;
        case APOSTROPHE: return VK_OEM_7;
        case SPACEBAR:   return VK_SPACE;
        case L_SHIFT:    return VK_LSHIFT;
        case R_SHIFT:    return VK_RSHIFT;
        case L_ALT:      return VK_LMENU;
        case R_ALT:      return VK_RMENU;
        case L_CTRL:     return VK_LCONTROL;
        case R_CTRL:     return VK_RCONTROL;
        case ESCAPE:     return VK_ESCAPE;
            // Function Keys //
        case F1:         return VK_F1;
        case F2:         return VK_F2;
        case F3:         return VK_F3;
        case F4:         return VK_F4;
        case F5:         return VK_F5;
        case F6:         return VK_F6;
        case F7:         return VK_F7;
        case F8:         return VK_F8;
        case F9:         return VK_F9;
        case F10:        return VK_F10;
        case F11:        return VK_F11;
        case F12:        return VK_F12;
        }
        return 0x00;
    }

    bool IsKeyPressed(LS::Input::KEYBOARD input)
    {
        return GetAsyncKeyState(ToWindowsKey(input)) & 0x8000;
    }
    
    bool IsMousePress(LS::Input::MOUSE_BUTTON input)
    {
        using enum LS::Input::MOUSE_BUTTON;
        switch (input)
        {
        case LMB:
            return GetAsyncKeyState(VK_LBUTTON) & 0x8000;
        case MMB:
            return GetAsyncKeyState(VK_MBUTTON) & 0x8000;
        case RMB:
            return GetAsyncKeyState(VK_RBUTTON) & 0x8000;
        default:
            return false;
        }
    }

    class Win32Window final : public LS::LSWindowBase
    {
    public:
        
        Win32Window(uint32_t width, uint32_t height, std::wstring_view title) noexcept
            : LS::LSWindowBase(width, height, title)
        {
            Initialize(width, height, title);
        }

        ~Win32Window() = default;

        // Inherited via LSWindowBase
        void ClearDisplay() noexcept final;
        void Show() noexcept final;
        void Close() noexcept final;
        void PollEvent() noexcept final;
        LRESULT HandleWinMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

        HWND Hwnd()
        {
            return m_hwnd;
        }

        HWND Hwnd() const
        {
            return m_hwnd;
        }

        void SetWndProcHandler(WndProcHandler&& handler)
        {
            m_wndProcHandler = std::move(handler);
        }

        WndProcHandler GetWndProc()
        {
            return m_wndProcHandler;
        }

    private:
        HINSTANCE m_hInstance;
        HWND m_hwnd;
        uint32_t m_prevWidth = 0u;
        uint32_t m_prevHeight = 0u;
        bool m_bIsResizing = false;
        bool m_bIsMinimized = false;
        MSG m_msg;
        HBRUSH m_bgBrush;
        WndProcHandler m_wndProcHandler;
        void Initialize(uint32_t width, uint32_t height, std::wstring_view title);
        void OnKeyPress(WPARAM wp);
        void OnKeyRelease(WPARAM wp);
        void GetScreenCoordinates(LPARAM lp, int& x, int& y);
        void OnMouseMove(uint32_t x, uint32_t y);
        void OnMouseClick(UINT msg, int x, int y);
        void OnMouseRelease(UINT msg, int x, int y);
        void OnMouseWheelScroll(short zDelta, int x, int y);
        int ToWindowsKey(LS::Input::KEYBOARD key);
        void TrackMouse(HWND hwnd);
    };
}

module : private;

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
    void Win32Window::ClearDisplay() noexcept
    {
        m_bgBrush = CreateSolidBrush(RGB(m_bgColor.R * 255, m_bgColor.G * 255, m_bgColor.B * 255));
        InvalidateRect(m_hwnd, NULL, TRUE);
    }

    void Win32Window::Show() noexcept
    {
        ShowWindow(reinterpret_cast<HWND>(m_winHandle), SW_SHOW);
        m_bIsOpen = true;
    }

    void Win32Window::Close() noexcept
    {
        PostQuitMessage(0);
        m_bIsOpen = false;;
    }

    void Win32Window::PollEvent() noexcept
    {
        if (PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&m_msg);
            DispatchMessage(&m_msg);
        }
    }

    LRESULT Win32Window::HandleWinMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        if (!m_bIsOpen)
            return DefWindowProc(hwnd, message, wparam, lparam);
        //TODO: For some reason the latest runs with MSVC makes this piece of code run when my std::function
        // object is null and it passes this check. I don't recall if this ran before or after my VS updated
        // probably after, so I'm guessing it broke it somehow. I'll check the code later, but I do want to redo this
        // window stuff because I'd rather try and implement ImGui and be window agnostic a bit. I'd rather create
        // a new library for Window management or something, but that will come later after my work on getting a basic
        // DX12 renderer setup
        /*LRESULT handled = 0;
        if (m_wndProcHandler)
        {
            handled = m_wndProcHandler(hwnd, message, wparam, lparam);
        }
        if (handled > 0)
            return handled;*/

        switch (message)
        {
        case (WM_NCCREATE):
        {
        }
        case (WM_PAINT):
        {
            break;
        }
        case (WM_CLOSE):
        {
            if (m_onWindowEvent)
                m_onWindowEvent(LS::WINDOW_EVENT::CLOSE_WINDOW);
            DestroyWindow(m_hwnd);
            m_bIsOpen = false;
            break;
        }
        case(WM_DESTROY):
        {
            if (m_onWindowEvent)
                m_onWindowEvent(LS::WINDOW_EVENT::CLOSE_WINDOW);
            PostQuitMessage(0);
            m_bIsOpen = false;
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
            m_mousePos.x = x;
            m_mousePos.y = y;
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
            m_mousePos.x = x;
            m_mousePos.y = y;
            break;
        }
        case(WM_MOUSEMOVE):
        {
            int x;
            int y;
            GetScreenCoordinates(lparam, x, y);
            OnMouseMove((uint32_t)x, (uint32_t)y);
            m_mousePos.x = x;
            m_mousePos.y = y;
            break;
        }
        case(WM_MOUSELEAVE):
        {
            if (m_onWindowEvent)
            {
                m_onWindowEvent(LS::WINDOW_EVENT::LEAVE_WINDOW);
            }
            break;
        }
        case(WM_MOUSEHOVER):
        {
            //TODO: Requires us to do some tracking to get this event. Look up MSDN for info
            if (m_onWindowEvent)
            {
                m_onWindowEvent(LS::WINDOW_EVENT::ENTER_WINDOW);
            }
            break;
        }
        case(WM_MOUSEWHEEL):
        {
            auto zDelta = GET_WHEEL_DELTA_WPARAM(wparam);
            int x;
            int y;
            //TODO: Invalid, need to look up how to obtain coordinates correctly because the LPARAM
            // looks lik it may not have the correct coordiantes
            GetScreenCoordinates(lparam, x, y);
            OnMouseWheelScroll(zDelta, x, y);
            break;
        }
        case(WM_SIZE):
        {
            m_width = GET_X_LPARAM(lparam);
            m_height = GET_Y_LPARAM(lparam);
            if (wparam == SIZE_MAXIMIZED)
            {
                if (m_bIsMinimized)
                {
                    m_bIsMinimized = false;
                }
                if (m_onWindowEvent)
                    m_onWindowEvent(LS::WINDOW_EVENT::MAXIMIZED_WINDOW);
            }
            else if (wparam == SIZE_MINIMIZED)
            {
                if (!m_bIsMinimized)
                {
                    m_bIsMinimized = true;
                }
                if (m_onWindowEvent)
                    m_onWindowEvent(LS::WINDOW_EVENT::MINIMIZED_WINDOW);
            }
            else if (wparam == SIZE_RESTORED)
            {
                if (m_bIsResizing)
                {
                    // Window was being resized by user, now they have stopped resizing the window
                    m_bIsResizing = false;
                    if (m_onWindowEvent)
                        m_onWindowEvent(LS::WINDOW_EVENT::WINDOW_RESIZE_END);
                }
                if (m_bIsMinimized)
                {
                    // Window was minimized but is now being restored out from minimization
                    m_bIsMinimized = false;
                    if (m_onWindowEvent)
                        m_onWindowEvent(LS::WINDOW_EVENT::RESTORED_WINDOW);
                }
            }
            else
            {
                if (m_onWindowEvent)
                    m_onWindowEvent(LS::WINDOW_EVENT::WINDOW_RESIZE_END);
            }
            break;
        }
        case (WM_SIZING):
        {
            m_bIsResizing = true;
            if (m_onWindowEvent)
                m_onWindowEvent(LS::WINDOW_EVENT::WINDOW_RESIZE_START);
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
            break;
        }
        case (WM_KILLFOCUS):
        {
            if (m_onWindowEvent)
            {
                m_onWindowEvent(LS::WINDOW_EVENT::LOST_FOCUS);
            }
            break;
        }
        case (WM_SETFOCUS):
        {
            if (m_onWindowEvent)
            {
                m_onWindowEvent(LS::WINDOW_EVENT::GAIN_FOCUS);
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
        auto style = WS_OVERLAPPEDWINDOW;

        RECT rt;
        rt.left = 0;
        rt.top = 0;
        rt.right = width;
        rt.bottom = height;
        // AdjustWindowRectExForDPI may also be worth considering //
        auto hresult = AdjustWindowRectEx(&rt, style, false, 0);

        if (!hresult)
        {
            auto error = GetLastError();
            auto eResult = HRESULT_FROM_WIN32(error);
            auto wformat = std::format(L"Failed to find window rect size. Error: {}", eResult);

            MessageBox(nullptr, wformat.c_str(), L"An Error Ocurred", MB_OK | MB_ICONERROR);
            throw std::runtime_error("An error ocurred");
        }

        auto newWidth = rt.right - rt.left;
        auto newHeight = rt.bottom - rt.top;
        auto handle = CreateWindowEx(0,
            title.data(),
            title.data(),
            style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            newWidth, newHeight,
            NULL,
            NULL,
            reinterpret_cast<HINSTANCE>(m_hInstance),
            this);

        if (!handle)
        {
            auto result = HRESULT_FROM_WIN32(GetLastError());
            auto format = std::format("Failed to create Window: {}", result);
            throw std::runtime_error(format.c_str());
        }

        m_winHandle = reinterpret_cast<LSWindowHandle>(handle);
        m_hwnd = handle;
    }

    void Win32Window::OnKeyPress(WPARAM wp)
    {
        if (!m_onKeyDown)
            return;

        m_onKeyDown(ToInputKey(wp));
    }

    void Win32Window::OnKeyRelease(WPARAM wp)
    {
        if (!m_onKeyUp)
            return;

        m_onKeyUp(ToInputKey(wp));
    }

    void Win32Window::GetScreenCoordinates(LPARAM lp, int& x, int& y)
    {
        x = GET_X_LPARAM(lp);
        y = GET_Y_LPARAM(lp);
    }

    void Win32Window::OnMouseMove(uint32_t x, uint32_t y)
    {
        TrackMouse(reinterpret_cast<HWND>(m_winHandle));
        if (!m_onCursorMove)
            return;

        m_onCursorMove(x, y);
    }

    void Win32Window::OnMouseClick(UINT msg, int x, int y)
    {
        if (!m_onMouseDown)
            return;
        using M = LS::Input::MOUSE_BUTTON;

        M input;
        switch (msg)
        {
        case WM_LBUTTONDOWN:
            input = M::LMB;
            break;
        case WM_MBUTTONDOWN:
            input = M::MMB;
            break;
        case WM_RBUTTONDOWN:
            input = M::RMB;
            break;
        default:
            input = M::NONE;
            break;
        }
        Input::InputMouseDown md{ .X = (uint32_t)x, .Y = (uint32_t)y, .Button = input, .IsHandled = false };
        m_onMouseDown(md);
    }

    void Win32Window::OnMouseRelease(UINT msg, int x, int y)
    {
        if (!m_onMouseUp)
            return;
        using M = LS::Input::MOUSE_BUTTON;

        M input;
        switch (msg)
        {
        case WM_LBUTTONUP:
            input = M::LMB;
            break;
        case WM_MBUTTONUP:
            input = M::MMB;
            break;
        case WM_RBUTTONUP:
            input = M::RMB;
            break;
        default:
            input = M::NONE;
            break;
        }
        Input::InputMouseUp md{ .X = (uint32_t)x, .Y = (uint32_t)y, .Button = input, .IsHandled = false };
        m_onMouseUp(md);
    }

    void Win32Window::OnMouseWheelScroll(short zDelta, int x, int y)
    {
        if (!m_onMWScroll)
            return;

        double diff = zDelta / 120.0f;

        Input::InputMouseWheelScroll input{ .X = (uint32_t)x, .Y = (uint32_t)y, .Delta = diff, .IsHandled = false };
        m_onMWScroll(input);
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
}

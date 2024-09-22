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
        auto PollEvent2() noexcept -> WINDOW_EVENT final;
        void SetCursorVisible(bool isVisible) noexcept final;
        void SetCursorClamp(bool isClamped) noexcept final;
        LRESULT HandleWinMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

        HWND Hwnd()
        {
            return m_hwnd;
        }

        const HWND Hwnd() const
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
        void TrackMouse();
        void BindCursor();
        void UnbindCursor();
        void HideCursor();
        void RevealCursor();
        auto TranslateWinMsg(UINT message, WPARAM wparam, LPARAM lparam) const noexcept -> WINDOW_EVENT;
    };
}

module : private;
import Win32.Utils;

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

    auto Win32Window::PollEvent2() noexcept -> WINDOW_EVENT
    {
        if (PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))
        {
            auto event = TranslateWinMsg(m_msg);
            TranslateMessage(&m_msg);
            DispatchMessage(&m_msg);
        }
    }

    void Win32Window::SetCursorVisible(bool isVisible) noexcept
    {
        if (isVisible)
        {
            RevealCursor();
        }
        else
        {
            HideCursor();
        }
    }

    void Win32Window::SetCursorClamp(bool isClamped) noexcept
    {
        if (isClamped)
        {
            BindCursor();
        }
        else
        {
            UnbindCursor();
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
        LRESULT handled = 0;
        if (m_wndProcHandler)
        {
            handled = m_wndProcHandler(hwnd, message, wparam, lparam);
        }
        if (handled > 0)
            return handled;

        switch (message)
        {
        case (WM_NCCREATE):
        {
            break;
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

        m_onKeyDown(ToLSKey(wp));
    }

    void Win32Window::OnKeyRelease(WPARAM wp)
    {
        if (!m_onKeyUp)
            return;

        m_onKeyUp(ToLSKey(wp));
    }

    void Win32Window::GetScreenCoordinates(LPARAM lp, int& x, int& y)
    {
        x = GET_X_LPARAM(lp);
        y = GET_Y_LPARAM(lp);
    }

    void Win32Window::OnMouseMove(uint32_t x, uint32_t y)
    {
        TrackMouse();
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

    void Win32Window::TrackMouse()
    {
        TRACKMOUSEEVENT tme{};
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.dwHoverTime = 1;
        tme.hwndTrack = m_hwnd;
        TrackMouseEvent(&tme);
    }

    void Win32Window::BindCursor()
    {
        if (!m_hwnd)
            return;
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        POINT pt{ .x = rc.left, .y = rc.top };
        POINT pt2{ .x = rc.right, .y = rc.bottom };
        ClientToScreen(m_hwnd, &pt);
        ClientToScreen(m_hwnd, &pt2);
        SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);

        ClipCursor(&rc);
    }

    void Win32Window::UnbindCursor()
    {
        ClipCursor(NULL);
    }

    void Win32Window::HideCursor()
    {
        ShowCursor(false);
    }

    void Win32Window::RevealCursor()
    {
        ShowCursor(true);
    }

    auto Win32Window::TranslateWinMsg(UINT message, WPARAM wparam, LPARAM lparam) const noexcept -> WINDOW_EVENT
    {
        using enum class LS::WINDOW_EVENT;
        switch (message)
        {
        case (WM_NCCREATE):
            return CREATED;
        case (WM_PAINT):
            return PAINT;
        case (WM_CLOSE):
            return CLOSE_WINDOW;
        case(WM_DESTROY):
            return SHUTDOWN_WINDOW;
        case(WM_KEYDOWN):
            return KEYDOWN;
        case (WM_KEYUP):
            return KEYUP;
        case(WM_LBUTTONDOWN):
            return MOUSE_LBUTTON_DOWN;
        case(WM_MBUTTONDOWN):
            return MOUSE_MBUTTON_DOWN;
        case(WM_RBUTTONDOWN):
            return MOUSE_RBUTTON_DOWN;
        case(WM_LBUTTONUP):
            return MOUSE_LBUTTON_UP;
        case(WM_MBUTTONUP):
            return MOUSE_MBUTTON_UP;
        case(WM_RBUTTONUP):
            return MOUSE_RBUTTON_UP;
        case(WM_MOUSEMOVE):
            return MOUSE_MOVE;
        case(WM_MOUSELEAVE):
            return MOUSE_LEAVE;
        case(WM_MOUSEHOVER):
            return MOUSE_HOVER;
        case(WM_MOUSEWHEEL):
            return MOUSE_WHEEL;
        case(WM_SIZE):
        {
            if (wparam == SIZE_MAXIMIZED)
            {
                return MAXIMIZED_WINDOW;
            }
            else if (wparam == SIZE_MINIMIZED)
            {
                return MINIMIZED_WINDOW;
            }
            else if (wparam == SIZE_RESTORED)
            {
                return RESTORED_WINDOW;
            }

            return WINDOW_RESIZE_END;
        }
        case (WM_SIZING):
            return WINDOW_RESIZE_START;
        case (WM_ENTERSIZEMOVE):
            return WINDOW_MOVE_START;
        case (WM_EXITSIZEMOVE):
            return WINDOW_MOVE_END;
        case (WM_KILLFOCUS):
            return LOST_FOCUS;
        case (WM_SETFOCUS):
            return GAIN_FOCUS;
        default:
            return UNKNOWN;
        }
    }
}

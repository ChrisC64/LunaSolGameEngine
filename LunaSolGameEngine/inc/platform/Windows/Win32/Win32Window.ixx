module;
#include <cstdint>
#include <string_view>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <functional>
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
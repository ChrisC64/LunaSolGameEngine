module;
#include <string>
#include <filesystem>
#include <array>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h> 
#include <windowsx.h>
export module Win32.Utils;
import Engine.Input;
import Engine.Logger;

export namespace LS::Win32
{
    [[nodiscard]] inline std::string HrToString(HRESULT hr)
    {
        return fmt::format("HRESULT of {:#x}", static_cast<UINT>(hr));
    }

    [[nodiscard]] inline std::wstring HrToWString(HRESULT hr)
    {
        return fmt::format(L"HRESULT of {:#x}", static_cast<UINT>(hr));
    }

    [[nodiscard]] inline HANDLE CreateEventHandler(LPCWSTR name = nullptr, BOOL isManualReset = FALSE, BOOL isSignaled = FALSE) noexcept
    {
        HANDLE eventHandle;
        eventHandle = ::CreateEventW(NULL, isManualReset, isSignaled, name);
        assert(eventHandle && "Failed to create fence event.");

        return eventHandle;
    }

    [[nodiscard]]
    inline auto FindModuleDir() -> std::filesystem::path
    {
        std::array<wchar_t, _MAX_PATH> modulePath;
        auto result = GetModuleFileName(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size()));
        if (result == 0)
        {
            LS::Log::TraceError(L"Failed to find module directory");
            return std::filesystem::path();
        }

        auto path = std::filesystem::path(std::begin(modulePath), std::end(modulePath));
        return path.parent_path();
    }
    
    [[nodiscard]]
    LS::Input::KEYBOARD ToLSKey(WPARAM wparam)
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

    [[nodiscard]]
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

    [[nodiscard]]
    bool IsKeyPressed(LS::Input::KEYBOARD input)
    {
        return GetAsyncKeyState(ToWindowsKey(input)) & 0x8000;
    }

    [[nodiscard]]
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

    [[nodiscard]]
    LS::Input::MODIFIERS GetModKeys()
    {
        using namespace LS::Input;
        using enum MODIFIERS;
        auto press = 0x8000;
        const auto B_ALT = 0x1;
        const auto B_CTRL = 0x2;
        const auto B_SHIFT = 0x4;
        const auto MASK = B_ALT | B_CTRL | B_SHIFT;
        char modState = 0;
        // ALT
        if (GetAsyncKeyState(VK_MENU) & press)
        {
            modState = B_ALT;
        }
        if (GetAsyncKeyState(VK_CONTROL) & press)
        {
            modState |= B_CTRL;
        }
        if (GetAsyncKeyState(VK_SHIFT) & press)
        {
            modState |= B_SHIFT;
        }

        if ((modState & MASK) == (B_ALT | B_CTRL | B_SHIFT))
            return CTRL_ALT_SHIFT;
        if ((modState & MASK) == (B_ALT | B_CTRL ))
            return CTRL_ALT;
        if ((modState & MASK) == (B_CTRL | B_SHIFT))
            return CTRL_SHIFT;
        if ((modState & MASK) == (B_ALT | B_SHIFT))
            return ALT_SHIFT;
        if ((modState & MASK) == B_ALT)
            return ALT;
        if ((modState & MASK) == B_SHIFT)
            return SHIFT;
        if ((modState & MASK) == B_CTRL)
            return CTRL;
        return NONE;
    }

    [[nodiscard]]
    LS::Input::MOUSE_BUTTON ToMouseButton(UINT msg)
    {
        using enum LS::Input::MOUSE_BUTTON;
        switch (msg)
        {
        case WM_LBUTTONDOWN:
            return LMB;
        case WM_MBUTTONDOWN:
            return MMB;
        case WM_RBUTTONDOWN:
            return RMB;
        default:
            return NONE;
        }
    }

    void GetScreenCoordinates(LPARAM lparam, int& x, int& y)
    {
        x = GET_X_LPARAM(lparam);
        y = GET_Y_LPARAM(lparam);
    }

    void GetNormalizedClientCoords(HWND hwnd, LPARAM lparam, double& x, double& y)
    {
        RECT rect;
        if (!GetClientRect(hwnd, &rect))
        {
            x = -1.0;
            y = -1.0;
            return;
        }
        int a, b;
        GetScreenCoordinates(lparam, a, b);
        x = a / (double)rect.right;
        y = b / (double)rect.bottom;
    }
}
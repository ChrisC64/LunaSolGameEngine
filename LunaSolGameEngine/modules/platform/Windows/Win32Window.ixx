module;
#include <cstdint>
#include <string_view>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <functional>
export module Platform.Win32Window;
import LSEDataLib;
import Engine.LSWindow;

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
        int ToWindowsKey(LS::LS_INPUT_KEY key);
        LS::LS_INPUT_KEY ToInputKey(WPARAM wparam);
        void TrackMouse(HWND hwnd);
    };
}
module;
#include "LSEFramework.h"

export module Platform.Win32Window;

import Engine.LSWindow;
//TODO: My idea is to see if I can restrict access to outside users. The window
// that is created would just be based off the build type, normally this is accomplished with
// preprocessor directives and what's included or not. Perhaps this isn't something I need to do?
// I can just control which files are compiled with what build I suppose, instead. 
namespace LS
{
    namespace Win32
    {
        class Win32Window;
    };

    export Ref<LS::Win32::Win32Window> LSCreateWindow(uint32_t width, uint32_t height,
        std::wstring_view title)
    {
        return std::make_unique<LS::Win32::Win32Window>(width, height, title);
    }
}

namespace LS::Win32
{
    //TODO: Would I want to make this so I can have the user only see the above LSCreateWindow()
    // and yet still construct this for the object I need in the Win32Window.cpp file where we
    // grab the pointer to the class so we can use the class' internal WndProc handler. 
    export class Win32Window : public LS::LSWindowBase
    {
    public:
        
        Win32Window(uint32_t width, uint32_t height, std::wstring_view title) noexcept
            : LS::LSWindowBase(width, height, title)
        {
            Initialize(width, height, title);
        }

        ~Win32Window() = default;

        // Inherited via LSWindowBase
        virtual void Show() override;
        virtual void Close() override;
        virtual void PollEvent() override;
        virtual LSWindowHandle GetHandleToWindow() const override;
        LRESULT HandleWinMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    private:
        HINSTANCE m_hInstance;
        uint32_t m_prevWidth = 0u;
        uint32_t m_prevHeight = 0u;
        bool m_bIsResizing = false;
        MSG m_msg;
        void Initialize(uint32_t width, uint32_t height, std::wstring_view title);
        void OnKeyPress(WPARAM wp);
        void OnKeyRelease(WPARAM wp);
        inline void GetScreenCoordinates(LPARAM lp, int& x, int& y);
        void OnMouseMove(int x, int y);
        void OnMouseClick(UINT msg, int x, int y);
        void OnMouseRelease(UINT msg, int x, int y);
        void OnMouseWheelScroll(short zDelta);
        int ToWindowsKey(LS::LS_INPUT_KEY key);
        LS::LS_INPUT_KEY ToInputKey(WPARAM wparam);
        void TrackMouse(HWND hwnd);
        void ResizeWindow(uint32_t width, uint32_t height);
    };
}
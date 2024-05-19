module;
#include <functional>
#include <string>

export module Engine.LSWindow;
import LSEDataLib;

export namespace LS
{
    struct MousePos
    {
        uint32_t x, y;
    };

    class LSWindowBase
    {
    public:
        virtual ~LSWindowBase() = default;

        bool IsOpen() const
        {
            return m_bIsOpen;
        }

        const uint32_t GetWidth() const
        {
            return m_width;
        }

        const uint32_t GetHeight() const
        {
            return m_height;
        }

        const MousePos GetMousePos() const
        {
            return m_mousePos;
        }

        const std::wstring_view GetTitle() const
        {
            return m_title;
        }

        void SetBackgroundColor(RGBA color)
        {
            m_bgColor = color;
        }

        /**
         * @brief Fills the window with the backgroudn color
        */
        virtual void ClearDisplay() noexcept = 0;
        /**
          * @brief Display the window to the user
         */
        virtual void Show() noexcept = 0;

        /*******
         * @brief Closes the window, destroying this and freeing resources it may have had.
        */
        virtual void Close() noexcept = 0;

        /**
          * @brief Continuously poll for events in this window's event queue.
        */
        virtual void PollEvent() noexcept = 0;

        /**
          * @brief Obtain the window's handle.
          * @return void* pointer handle
         */
        auto GetHandleToWindow() const -> LSWindowHandle
        {
            return m_winHandle;
        }

        void RegisterKeyboardDown(LSOnKeyboardDown keyboardDown)
        {
            m_onKeyDown = keyboardDown;
        }

        void RegisterKeyboardUp(LSOnKeyboardUp keyboardUp)
        {
            m_onKeyUp = keyboardUp;
        }

        void RegisterMouseDown(LSOnMouseDown mouseDown)
        {
            m_onMouseDown = mouseDown;
        }

        void RegisterMouseUp(LSOnMouseUp mouseUp)
        {
            m_onMouseUp = mouseUp;
        }

        void RegisterMouseWheel(LSOnMouseWheelScroll mouseWheel)
        {
            m_onMWScroll = mouseWheel;
        }

        void RegisterWindowEventCallback(OnWindowEvent callback)
        {
            m_onWindowEvent = callback;
        }

        void RegisterMouseMoveCallback(LSOnMouseMove callback)
        {
            m_onCursorMove = callback;
        }

    protected:
        std::wstring m_title;
        uint32_t m_width;
        uint32_t m_height;
        bool m_bIsOpen = false;

        LSOnMouseMove m_onCursorMove;
        LSOnKeyboardDown m_onKeyDown;
        LSOnKeyboardUp m_onKeyUp;
        LSOnMouseDown m_onMouseDown;
        LSOnMouseUp m_onMouseUp;
        LSOnMouseWheelScroll m_onMWScroll;

        LSWindowHandle m_winHandle;
        OnWindowEvent m_onWindowEvent;
        //@brief The color to fill the window background
        RGBA m_bgColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        MousePos m_mousePos{ 0, 0 };
        LSWindowBase(uint32_t width, uint32_t height, std::wstring_view  title) : m_width(width),
            m_height(height),
            m_title(title),
            m_bIsOpen(false)
        {
        }
    };
}
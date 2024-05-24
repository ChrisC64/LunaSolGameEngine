module;
#include <functional>
#include <string>

export module Engine.LSWindow;
import Engine.Input;
import Engine.Defines;

export namespace LS
{
    struct MousePos
    {
        uint32_t x, y;
    };

    enum class WINDOW_EVENT : int32_t
    {
        LOST_FOCUS = 0, // @brief window is no longer active window
        GAIN_FOCUS, // @brief window is active 
        LEAVE_WINDOW, // @brief cursor moves out of window region
        ENTER_WINDOW, // @brief cursor has moved back into window region
        CLOSE_WINDOW, // @brief the window has been closed
        MINIMIZED_WINDOW, //@brief the window has been minimized
        RESTORED_WINDOW, //@brief the window has been restored from minimized
        MAXIMIZED_WINDOW, //@brief the window has been maximized
        WINDOW_RESIZE_START, //@brief the start of a window resize event
        WINDOW_RESIZE_END, //@brief the window has been resized
        WINDOW_MOVE_START, //@brief the window is being moved
        WINDOW_MOVE_END, //@brief the window has finished moving
    };

    /**
    * @brief The Window Event Callback
    *
    * Returns events that are important to those who manage the Window
    * @code
    * void function_name(@link LS::WINDOW_EVENT ev)
    * @code
    * @param @link LS::WINDOW_EVENT the event
    */
    using OnWindowEvent = std::function<void(WINDOW_EVENT ev)>;

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

        void SetBackgroundColor(Colors::RGBA color)
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

        void RegisterKeyboardDown(Input::LSOnKeyboardDown keyboardDown)
        {
            m_onKeyDown = keyboardDown;
        }

        void RegisterKeyboardUp(Input::LSOnKeyboardUp keyboardUp)
        {
            m_onKeyUp = keyboardUp;
        }

        void RegisterMouseDown(Input::LSOnMouseDown mouseDown)
        {
            m_onMouseDown = mouseDown;
        }

        void RegisterMouseUp(Input::LSOnMouseUp mouseUp)
        {
            m_onMouseUp = mouseUp;
        }

        void RegisterMouseWheel(Input::LSOnMouseWheelScroll mouseWheel)
        {
            m_onMWScroll = mouseWheel;
        }

        void RegisterWindowEventCallback(OnWindowEvent callback)
        {
            m_onWindowEvent = callback;
        }

        void RegisterMouseMoveCallback(Input::LSOnMouseMove callback)
        {
            m_onCursorMove = callback;
        }

    protected:
        std::wstring m_title;
        uint32_t m_width;
        uint32_t m_height;
        bool m_bIsOpen = false;

        Input::LSOnMouseMove m_onCursorMove;
        Input::LSOnKeyboardDown m_onKeyDown;
        Input::LSOnKeyboardUp m_onKeyUp;
        Input::LSOnMouseDown m_onMouseDown;
        Input::LSOnMouseUp m_onMouseUp;
        Input::LSOnMouseWheelScroll m_onMWScroll;

        LSWindowHandle m_winHandle;
        OnWindowEvent m_onWindowEvent;
        //@brief The color to fill the window background
        Colors::RGBA m_bgColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        MousePos m_mousePos{ 0, 0 };
        LSWindowBase(uint32_t width, uint32_t height, std::wstring_view  title) : m_width(width),
            m_height(height),
            m_title(title),
            m_bIsOpen(false)
        {
        }
    };

}
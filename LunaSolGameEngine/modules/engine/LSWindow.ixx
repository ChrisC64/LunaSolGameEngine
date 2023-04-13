module;
#include "LSEFramework.h"

export module Engine.LSWindow;
import Data.LSWindow.Types;
import Data.LSDataTypes;

export namespace LS
{
    class LSWindowBase
    {
    public:
        virtual ~LSWindowBase() = default;

        bool IsOpen() const
        {
            return m_bIsOpen;
        }

        uint32_t GetWidth() const
        {
            return m_width;
        }

        uint32_t GetHeight() const
        {
            return m_height;
        }

        const std::wstring_view GetTitle() const
        {
            return m_title;
        }

        void SetBackgroundColor(ColorRGB color)
        {
            m_bgColor = std::move(color);
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
        virtual LSWindowHandle GetHandleToWindow() const = 0;

    protected:
        std::wstring m_title;
        uint32_t m_width;
        uint32_t m_height;
        bool m_bIsOpen = false;
        KeyboardCallback m_onKeyboardInput;
        MouseButtonCallback m_onMouseInput;
        CursorMoveCallback m_onCursorMove;
        MouseWheelScrollCallback m_onMouseWheelScroll;
        LSWindowHandle m_winHandle;
        OnWindowEvent m_onWindowEvent;
        //@brief The color to fill the window background
        ColorRGB m_bgColor{ 1.0f, 1.0f, 1.0f };

        LSWindowBase(uint32_t width, uint32_t height, std::wstring_view  title) : m_width(width),
            m_height(height),
            m_title(title),
            m_bIsOpen(false)
        {
        }

        // Callback Functions //
        void RegisterKeyboardCallback(KeyboardCallback callback)
        {
            m_onKeyboardInput = callback;
        }

        void RegisterMouseButtonCallback(MouseButtonCallback callback)
        {
            m_onMouseInput = callback;
        }

        void RegisterCursorMoveCallback(CursorMoveCallback callback)
        {
            m_onCursorMove = callback;
        }

        void RegisterMouseScrollCallback(MouseWheelScrollCallback callback)
        {
            m_onMouseWheelScroll = callback;
        }

        void RegisterWindowEventCallback(OnWindowEvent callback)
        {
            m_onWindowEvent = callback;
        }
    };
}
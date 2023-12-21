module;
#include <cstdint>
#include <array>
#include <vector>
#include <functional>

export module LSEDataLib:Input;

export namespace LS
{
    /***
     * @brief Modifier keys like CTRL, SHIFT, and ALT
     */
    enum class LS_INPUT_MODS : int16_t
    {
        NONE = 0x0,
        CTRL,
        ALT,
        SHIFT,
        CTRL_ALT,
        CTRL_SHIFT,
        CTRL_ALT_SHIFT,
        ALT_SHIFT
    };

    /***
     * @brief Actions or states the key can be in. Pressed, Release, or Hold.
     */
    enum class LS_INPUT_ACTION : int8_t
    {
        NONE = 0,
        PRESS,
        RELEASE,
    };

    /***
     * @brief Input Keys from the keyboard
     */
    enum class LS_INPUT_KEY : int32_t
    {
        // DEFAULT STATE //
        NONE = 0x00,
        // Alphabet
        A = 0x01, B = 0x02, C = 0x03, D = 0x04, E = 0x05, F = 0x06, G = 0x07, H = 0x08, I = 0x09, J = 0x0A,
        K = 0x0B, L = 0x0C, M = 0x0D, N = 0x0E, O = 0x0F, P = 0x10, Q = 0x11, R = 0x12, S = 0x13, T = 0x14,
        U = 0x15, V = 0x16, W = 0x17, X = 0x18, Y = 0x19, Z = 0x20,
        // ARROW KEYS
        AR_UP = 0x21, AR_DOWN = 0x22, AR_LEFT = 0x23, AR_RIGHT = 0x24,
        // Number Row
        D0 = 0x30, D1 = 0x31, D2 = 0x32, D3 = 0x33, D4 = 0x34, D5 = 0x35, D6 = 0x36, D7 = 0x37, D8 = 0x38, D9 = 0x39,
        // Number Pad
        NUM_0 = 0x40, NUM_1 = 0x41, NUM_2 = 0x42, NUM_3 = 0x43, NUM_4 = 0x44, NUM_5 = 0x45, NUM_6 = 0x46, NUM_7 = 0x47,
        NUM_8 = 0x48, NUM_9 = 0x49,
        // Special Characters,
        BACK_TICK = 0x50/* ` */, MINUS = 0x51/* - */, EQUALS = 0x52, /* = */ L_CURLY = 0x53/* { */,
        R_CURLY = 0x54/* } */, F_SLASH = 0x55/* / */, B_SLASH = 0x56/* \ */, SINGLE_QUOTE = 0x57, /* ' */
        COMMA = 0x58/* , */,
        PERIOD = 0x59/* . */,
        APOSTROPHE = 0x5A/* ; */,
        SPACEBAR = 0x5B, BACKSPACE = 0x5C, ENTER = 0x5D,
        ESCAPE = 0x5E, PLUS = 0x5F, /* + */
        UNDERSCORE = 0x60/* _ */,
        HOME = 0x61, END = 0x62, PAGE_UP = 0x63, PAGE_DN = 0x64, DEL = 0x65, INSERT = 0x66, TAB = 0x67,
        // Modifier Keys // 
        L_CTRL = 0xD0, R_CTRL = 0xD1, L_ALT = 0xD2, R_ALT = 0xD3, L_SHIFT = 0xD4, R_SHIFT = 0xD5,
        // Function Keys
        F1 = 0xA0, F2 = 0xA1, F3 = 0xA2, F4 = 0xA3, F5 = 0xA4, F6 = 0xA5, F7 = 0xA6, F8 = 0xA7, F9 = 0xA8,
        F10 = 0xA9, F11 = 0xAA, F12 = 0xAB, F13 = 0xAC, F14 = 0xAD, F15 = 0xAE, F16 = 0xAF,
        F17 = 0xB0, F18 = 0xB1, F19 = 0xB2, F20 = 0xB3
    };

    /***
     * @brief Input from the Mouse
     */
    enum class LS_INPUT_MOUSE : uint8_t
    {
        NONE = 0x0,
        LMB = 1 << 1, // @brief Left mouse button
        RMB = 1 << 2, // @brief Right mouse button
        MMB = 1 << 3, // @brief Middle mouse button
        MB4 = 1 << 4, // @brief Extra mouse button 4
        MB5 = 1 << 5, // @brief Extra mouse button 5
        MB6 = 1 << 6,
        S_WHEEL = 1 << 7// @brief Scrollwheel click
    };

    /***
     * @brief The input tye being used (mouse and keyboard support only at the moment)
     */
    enum class LS_INPUT_TYPE : uint16_t
    {
        MOUSE = 1 << 1,
        KEYBOARD = 1 << 2,
        GAMEPAD = 1 << 3
    };

    enum class WINDOW_SETTING
    {
        FULLSCREEN,
        WINDOW,
        BORDERLESS_WINDOW
    };

    enum class LS_WINDOW_EVENT : int32_t
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

    struct InputKeyDown
    {
        LS_INPUT_KEY Key = LS_INPUT_KEY::NONE;
        bool IsHandled = false;
    };

    struct InputKeyUp
    {
        LS_INPUT_KEY Key = LS_INPUT_KEY::NONE;
        bool IsHandled = false;
    };

    struct InputMouseDown
    {
        uint32_t X = 0u;
        uint32_t Y = 0u;
        LS_INPUT_MOUSE Button = LS_INPUT_MOUSE::NONE;
        bool IsHandled = false;
    };

    struct InputMouseUp
    {
        uint32_t X = 0u;
        uint32_t Y = 0u;
        LS_INPUT_MOUSE Button = LS_INPUT_MOUSE::NONE;
        bool IsHandled = false;
    };

    struct InputMouseWheelScroll
    {
        uint32_t X = 0u;
        uint32_t Y = 0u;
        double Delta = 0.0f;
        bool IsHandled = false;
    };

    // TYPEDEFS //
    /**
    * @brief The cursor move callback function signature
    *
    * A cursor move is fired when the mouse cursor is moved
    * @code
    * void function_name(double xPos, double yPos)
    * @endcode
    *
    * @param xPos the current X position of the cursor starting at 0 from the left
    * @param yPos the current Y position of the cursor starting at 0 from the top
    */
    using LSOnMouseMove = std::function<void(uint32_t x, uint32_t y)>;

    /**
     * @brief A mouse scroll wheel callback that passes over an integer value that is system dependent
    */
    using LSMouseWheelScrollCallback = std::function<void(int64_t delta)>;

    using LSOnKeyboardDown = std::function<void(const LS::InputKeyDown& input)>;
    using LSOnKeyboardUp = std::function<void(const LS::InputKeyUp& input)>;
    using LSOnMouseDown = std::function<void(const LS::InputMouseDown& input)>;
    using LSOnMouseUp = std::function<void(const LS::InputMouseUp& input)>;
    using LSOnMouseWheelScroll = std::function<void(const LS::InputMouseWheelScroll& input)>;

    /**
    * @brief The Window Event Callback
    *
    * Returns events that are important to those who manage the Window
    * @code
    * void function_name(LS::Core::LS_WINDOW_EVENT ev)
    * @code
    * @param LS::Core::LS_WINDOW_EVENT the event
    */
    using OnWindowEvent = std::function<void(LS_WINDOW_EVENT ev)>;

    using LSWindowHandle = void*;

    using LSAppInstance = void*;
}
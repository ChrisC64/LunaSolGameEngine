export module Engine.Input;
import <functional>;
import <array>;
export namespace LS::Input
{
    /***
     * @brief Modifier keys like CTRL, SHIFT, and ALT
     */
    enum class MODIFIERS : int16_t
    {
        CTRL = 0x1,
        ALT = 0x2,
        SHIFT = 0x4,
        CTRL_ALT = CTRL | ALT,
        CTRL_SHIFT = CTRL | SHIFT,
        CTRL_ALT_SHIFT = CTRL | ALT | SHIFT,
        ALT_SHIFT = ALT | SHIFT
    };

    /***
     * @brief Actions or states the key can be in. Pressed, Release, or Hold.
     */
    enum class STATE : int8_t
    {
        RELEASE = 0,
        PRESS = 1,
    };

    /***
     * @brief Input Keys from the keyboard
     */
    enum class KEYBOARD : int32_t
    {
        // DEFAULT STATE //
        NONE = 0,
        // Alphabet
        A, B, C, D, E, F, G, H, I, J,
        K, L, M, N, O, P, Q, R, S, T,
        U, V, W, X, Y, Z,
        // ARROW KEYS
        AR_UP, AR_DOWN, AR_LEFT, AR_RIGHT,
        // Number Row
        D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
        // Number Pad
        NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7,
        NUM_8, NUM_9,
        // Special Characters,
        BACK_TICK/* ` */, MINUS/* - */, EQUALS, /* = */ L_CURLY/* { */,
        R_CURLY/* } */, F_SLASH/* / */, B_SLASH/* \ */, SINGLE_QUOTE, /* ' */
        COMMA/* , */,
        PERIOD/* . */,
        APOSTROPHE/* ; */,
        SPACEBAR, BACKSPACE, ENTER,
        ESCAPE, PLUS, /* + */
        UNDERSCORE/* _ */,
        HOME, END, PAGE_UP, PAGE_DN, DEL, INSERT, TAB,
        // Modifier Keys // 
        L_CTRL, R_CTRL, L_ALT, R_ALT, L_SHIFT, R_SHIFT,
        // Function Keys
        F1, F2, F3, F4, F5, F6, F7, F8, F9,
        F10, F11, F12,
        INPUT_COUNT
    };

    /***
     * @brief Input from the Mouse
     */
    enum class MOUSE_BUTTON : uint32_t
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
    enum class TYPE : uint16_t
    {
        MOUSE,
        KEYBOARD,
        GAMEPAD
    };

    enum class WINDOW_SETTING : uint8_t
    {
        FULLSCREEN,
        WINDOW,
        BORDERLESS_WINDOW
    };

    struct InputMouseDown
    {
        uint32_t X = 0u;
        uint32_t Y = 0u;
        MOUSE_BUTTON Button = MOUSE_BUTTON::NONE;
        bool IsHandled = false;
    };

    struct InputMouseUp
    {
        uint32_t X = 0u;
        uint32_t Y = 0u;
        MOUSE_BUTTON Button = MOUSE_BUTTON::NONE;
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
    * @brief The cursor move callback function signature. Returns coordinates 
    * based on the Window's coordinate system with top left/top right being 0,0
    * to the screen's WIDTH/HEIGHT
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

    using LSOnKeyboardDown = std::function<void(KEYBOARD input)>;
    using LSOnKeyboardUp = std::function<void(KEYBOARD input)>;
    using LSOnMouseDown = std::function<void(const InputMouseDown& input)>;
    using LSOnMouseUp = std::function<void(const InputMouseUp& input)>;
    using LSOnMouseWheelScroll = std::function<void(const InputMouseWheelScroll& input)>;
}
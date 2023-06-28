module;
#include <cstdint>
#include <array>
#include <vector>
#include <functional>

export module Data.LSWindow.Types;

export namespace LS
{
	/***
	 * @brief Modifier keys like CTRL, SHIFT, and ALT
	 */
	enum class LS_INPUT_MODS : int16_t
	{
		NONE = 0x0,
		CTRL = 0x1,
		ALT = 0x2,
		SHIFT = 0x4,
	};

	/***
	 * @brief Actions or states the key can be in. Pressed, Release, or Hold.
	 */
	enum class LS_INPUT_ACTION : int8_t
	{
		NONE = 0,
		PRESS = 0x1,
		RELEASE = 0x2,
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
		SW_CLICK = 1 << 7// @brief Scrollwheel click
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
		GAIN_FOCUS = 1 << 1, // @brief window is active 
		LEAVE_WINDOW = 1 << 2, // @brief cursor moves out of window region
		ENTER_WINDOW = 1 << 3, // @brief cursor has moved back into window region
		RESIZE_WINDOW = 1 << 4, // @brief the window has changed size 
		CLOSE_WINDOW = 1 << 5, // @brief the window has been closed
		MINIMIZED_WINDOW = 1 << 6, //@brief the window has been minimized
		RESTORED_WINDOW = 1 << 7, //@brief the window has been restored from minimized
		MAXIMIZED_WINDOW = 1 << 8 //@brief the window has been maximized
	};

	struct InputKeyEvent
	{
		LS_INPUT_ACTION Action;
		LS_INPUT_KEY KeyInput;
		std::array<LS_INPUT_MODS, 3> Mods;
		bool IsHandled;
		bool IsRepeated;
	};

	struct InputMouseEvent
	{
		LS_INPUT_ACTION Action;
		int32_t X;
		int32_t Y;
		int32_t ScrollDelta;
		bool IsHandled;
		bool IsRepeated;
		std::vector<LS_INPUT_MOUSE> MouseInput;
		std::array<LS_INPUT_MODS, 3> Mods;
	};

	// TYPEDEFS //
	/**
	* @brief The keyboard callback signature for function pointers
	*
	* A keyboard callback is initiated when any key press or release
	* is made by the Window's messaging system.
	* @code
	* void function_name(int key, short mod, short action)
	* @endcode
	*
	* @param key They keyboard key belonging to @ref LS_INPUT_KEY
	* @param mod The modifier's being held by LS_INPUT_MODS
	* @param action The action of the key based on LS_INPUT_ACTION
	*/
	using KeyboardCallback = std::function<void(int32_t key, int16_t mods, int8_t action)>;

	/**
	* @brief The mouse button callback signature for function pointers
	*
	* A mouse callback is initiated when any key press or release
	* is made by the Window's messaging system.
	* @code
	* void function_name(int button, short mod, short action)
	* @endcode
	*
	* @param button They mouse button belonging to @ref LS_INPUT_MOUSE
	* @param mod The modifier's being held by LS_INPUT_MODS
	* @param action The action of the key based on LS_INPUT_ACTION
	* @param x the x coordinate of the screen click
	* @param y the y coordinate of the screen click
	*/
	using MouseButtonCallback= std::function<void(int32_t button, int32_t mod, int16_t action, int32_t x, int32_t y)>;

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
	using CursorMoveCallback = std::function<void(int32_t x, int32_t y)>;

	/**
	* @brief The scroll of the mouse wheel
	*
	* When the mouse wheel scrolls, this event is fired.
	* @code
	* void function_name(double zDelta)
	* @endcode
	*
	* @param zDelta the delta change of the scroll wheel
	*/
	using MouseWheelScrollCallback = std::function<void(double delta)>;

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
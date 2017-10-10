//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Math/BsVector2I.h"

namespace bs
{
	// Undefine conflicting defines from other libs
#undef None

	/** @addtogroup Input
	 *  @{
	 */

	/**
	 * Contains all possible input buttons, including keyboard scan codes, mouse buttons and gamepad buttons.
	 *
	 * @note
	 * These codes are only keyboard scan codes. This means that exact scan code identifier might not correspond to that 
	 * exact character on user's keyboard, depending on user's input locale. Only for US locale will these scan code names 
	 * match the actual keyboard input. Think of the US key code names as only a convenience for more easily identifying
	 * which location on the keyboard a scan code represents.
	 * @note
	 * When storing these sequentially make sure to only reference the low order 2 bytes. Two high order bytes are used for 
	 * various flags.
	 */
	enum ButtonCode : UINT32
	{
		BC_UNASSIGNED  = 0x00,
		BC_ESCAPE      = 0x01,
		BC_1           = 0x02,
		BC_2           = 0x03,
		BC_3           = 0x04,
		BC_4           = 0x05,
		BC_5           = 0x06,
		BC_6           = 0x07,
		BC_7           = 0x08,
		BC_8           = 0x09,
		BC_9           = 0x0A,
		BC_0           = 0x0B,
		BC_MINUS       = 0x0C,    // - on main keyboard
		BC_EQUALS      = 0x0D,
		BC_BACK        = 0x0E,    // backspace
		BC_TAB         = 0x0F,
		BC_Q           = 0x10,
		BC_W           = 0x11,
		BC_E           = 0x12,
		BC_R           = 0x13,
		BC_T           = 0x14,
		BC_Y           = 0x15,
		BC_U           = 0x16,
		BC_I           = 0x17,
		BC_O           = 0x18,
		BC_P           = 0x19,
		BC_LBRACKET    = 0x1A,
		BC_RBRACKET    = 0x1B,
		BC_RETURN      = 0x1C,    // Enter on main keyboard
		BC_LCONTROL    = 0x1D,
		BC_A           = 0x1E,
		BC_S           = 0x1F,
		BC_D           = 0x20,
		BC_F           = 0x21,
		BC_G           = 0x22,
		BC_H           = 0x23,
		BC_J           = 0x24,
		BC_K           = 0x25,
		BC_L           = 0x26,
		BC_SEMICOLON   = 0x27,
		BC_APOSTROPHE  = 0x28,
		BC_GRAVE       = 0x29,    // accent
		BC_LSHIFT      = 0x2A,
		BC_BACKSLASH   = 0x2B,
		BC_Z           = 0x2C,
		BC_X           = 0x2D,
		BC_C           = 0x2E,
		BC_V           = 0x2F,
		BC_B           = 0x30,
		BC_N           = 0x31,
		BC_M           = 0x32,
		BC_COMMA       = 0x33,
		BC_PERIOD      = 0x34,    // . on main keyboard
		BC_SLASH       = 0x35,    // / on main keyboard
		BC_RSHIFT      = 0x36,
		BC_MULTIPLY    = 0x37,    // * on numeric keypad
		BC_LMENU       = 0x38,    // left Alt
		BC_SPACE       = 0x39,
		BC_CAPITAL     = 0x3A,
		BC_F1          = 0x3B,
		BC_F2          = 0x3C,
		BC_F3          = 0x3D,
		BC_F4          = 0x3E,
		BC_F5          = 0x3F,
		BC_F6          = 0x40,
		BC_F7          = 0x41,
		BC_F8          = 0x42,
		BC_F9          = 0x43,
		BC_F10         = 0x44,
		BC_NUMLOCK     = 0x45,
		BC_SCROLL      = 0x46,    // Scroll Lock
		BC_NUMPAD7     = 0x47,
		BC_NUMPAD8     = 0x48,
		BC_NUMPAD9     = 0x49,
		BC_SUBTRACT    = 0x4A,    // - on numeric keypad
		BC_NUMPAD4     = 0x4B,
		BC_NUMPAD5     = 0x4C,
		BC_NUMPAD6     = 0x4D,
		BC_ADD         = 0x4E,    // + on numeric keypad
		BC_NUMPAD1     = 0x4F,
		BC_NUMPAD2     = 0x50,
		BC_NUMPAD3     = 0x51,
		BC_NUMPAD0     = 0x52,
		BC_DECIMAL     = 0x53,    // . on numeric keypad
		BC_OEM_102     = 0x56,    // < > | on UK/Germany keyboards
		BC_F11         = 0x57,
		BC_F12         = 0x58,
		BC_F13         = 0x64,    //                     (NEC PC98)
		BC_F14         = 0x65,    //                     (NEC PC98)
		BC_F15         = 0x66,    //                     (NEC PC98)
		BC_KANA        = 0x70,    // (Japanese keyboard)
		BC_ABNT_C1     = 0x73,    // / ? on Portugese (Brazilian) keyboards
		BC_CONVERT     = 0x79,    // (Japanese keyboard)
		BC_NOCONVERT   = 0x7B,    // (Japanese keyboard)
		BC_YEN         = 0x7D,    // (Japanese keyboard)
		BC_ABNT_C2     = 0x7E,    // Numpad . on Portugese (Brazilian) keyboards
		BC_NUMPADEQUALS= 0x8D,    // = on numeric keypad (NEC PC98)
		BC_PREVTRACK   = 0x90,    // Previous Track (BC_CIRCUMFLEX on Japanese keyboard)
		BC_AT          = 0x91,    //                     (NEC PC98)
		BC_COLON       = 0x92,    //                     (NEC PC98)
		BC_UNDERLINE   = 0x93,    //                     (NEC PC98)
		BC_KANJI       = 0x94,    // (Japanese keyboard)
		BC_STOP        = 0x95,    //                     (NEC PC98)
		BC_AX          = 0x96,    //                     (Japan AX)
		BC_UNLABELED   = 0x97,    //                        (J3100)
		BC_NEXTTRACK   = 0x99,    // Next Track
		BC_NUMPADENTER = 0x9C,    // Enter on numeric keypad
		BC_RCONTROL    = 0x9D,
		BC_MUTE        = 0xA0,    // Mute
		BC_CALCULATOR  = 0xA1,    // Calculator
		BC_PLAYPAUSE   = 0xA2,    // Play / Pause
		BC_MEDIASTOP   = 0xA4,    // Media Stop
		BC_VOLUMEDOWN  = 0xAE,    // Volume -
		BC_VOLUMEUP    = 0xB0,    // Volume +
		BC_WEBHOME     = 0xB2,    // Web home
		BC_NUMPADCOMMA = 0xB3,    // , on numeric keypad (NEC PC98)
		BC_DIVIDE      = 0xB5,    // / on numeric keypad
		BC_SYSRQ       = 0xB7,
		BC_RMENU       = 0xB8,    // right Alt
		BC_PAUSE       = 0xC5,    // Pause
		BC_HOME        = 0xC7,    // Home on arrow keypad
		BC_UP          = 0xC8,    // UpArrow on arrow keypad
		BC_PGUP        = 0xC9,    // PgUp on arrow keypad
		BC_LEFT        = 0xCB,    // LeftArrow on arrow keypad
		BC_RIGHT       = 0xCD,    // RightArrow on arrow keypad
		BC_END         = 0xCF,    // End on arrow keypad
		BC_DOWN        = 0xD0,    // DownArrow on arrow keypad
		BC_PGDOWN      = 0xD1,    // PgDn on arrow keypad
		BC_INSERT      = 0xD2,    // Insert on arrow keypad
		BC_DELETE      = 0xD3,    // Delete on arrow keypad
		BC_LWIN        = 0xDB,    // Left Windows key
		BC_RWIN        = 0xDC,    // Right Windows key
		BC_APPS        = 0xDD,    // AppMenu key
		BC_POWER       = 0xDE,    // System Power
		BC_SLEEP       = 0xDF,    // System Sleep
		BC_WAKE        = 0xE3,    // System Wake
		BC_WEBSEARCH   = 0xE5,    // Web Search
		BC_WEBFAVORITES= 0xE6,    // Web Favorites
		BC_WEBREFRESH  = 0xE7,    // Web Refresh
		BC_WEBSTOP     = 0xE8,    // Web Stop
		BC_WEBFORWARD  = 0xE9,    // Web Forward
		BC_WEBBACK     = 0xEA,    // Web Back
		BC_MYCOMPUTER  = 0xEB,    // My Computer
		BC_MAIL        = 0xEC,    // Mail
		BC_MEDIASELECT = 0xED,    // Media Select
		BC_MOUSE_LEFT = 0x800000EE, // Mouse buttons - Most important bit signifies this key is a mouse button
		BC_MOUSE_RIGHT,
		BC_MOUSE_MIDDLE,
		BC_MOUSE_BTN4,
		BC_MOUSE_BTN5,
		BC_MOUSE_BTN6,
		BC_MOUSE_BTN7,
		BC_MOUSE_BTN8,
		BC_MOUSE_BTN9,
		BC_MOUSE_BTN10,
		BC_MOUSE_BTN11,
		BC_MOUSE_BTN12,
		BC_MOUSE_BTN13,
		BC_MOUSE_BTN14,
		BC_MOUSE_BTN15,
		BC_MOUSE_BTN16,
		BC_MOUSE_BTN17,
		BC_MOUSE_BTN18,
		BC_MOUSE_BTN19,
		BC_MOUSE_BTN20,
		BC_MOUSE_BTN21,
		BC_MOUSE_BTN22,
		BC_MOUSE_BTN23,
		BC_MOUSE_BTN24,
		BC_MOUSE_BTN25,
		BC_MOUSE_BTN26,
		BC_MOUSE_BTN27,
		BC_MOUSE_BTN28,
		BC_MOUSE_BTN29,
		BC_MOUSE_BTN30,
		BC_MOUSE_BTN31,
		BC_MOUSE_BTN32,
		BC_GAMEPAD_A = 0x4000010F, // Joystick/Gamepad buttons- Second most important bit signifies key is a gamepad button
		BC_GAMEPAD_B,              // Similar to keyboard names, these are for convenience named after Xbox controller buttons
		BC_GAMEPAD_X,              // but if some other controller is connected you will need to learn yourself which of these
		BC_GAMEPAD_Y,              // corresponds to which actual button on the controller.
		BC_GAMEPAD_LB,
		BC_GAMEPAD_RB,
		BC_GAMEPAD_LS,
		BC_GAMEPAD_RS,
		BC_GAMEPAD_BACK,
		BC_GAMEPAD_START,
		BC_GAMEPAD_DPAD_LEFT,
		BC_GAMEPAD_DPAD_RIGHT,
		BC_GAMEPAD_DPAD_UP,
		BC_GAMEPAD_DPAD_DOWN,
		BC_GAMEPAD_BTN1,
		BC_GAMEPAD_BTN2,
		BC_GAMEPAD_BTN3,
		BC_GAMEPAD_BTN4,
		BC_GAMEPAD_BTN5,
		BC_GAMEPAD_BTN6,
		BC_GAMEPAD_BTN7,
		BC_GAMEPAD_BTN8,
		BC_GAMEPAD_BTN9,
		BC_GAMEPAD_BTN10,
		BC_GAMEPAD_BTN11,
		BC_GAMEPAD_BTN12,
		BC_GAMEPAD_BTN13,
		BC_GAMEPAD_BTN14,
		BC_GAMEPAD_BTN15,
		BC_GAMEPAD_BTN16,
		BC_GAMEPAD_BTN17,
		BC_GAMEPAD_BTN18,
		BC_GAMEPAD_BTN19,
		BC_GAMEPAD_BTN20,
		BC_GAMEPAD_DPAD_UPLEFT,
		BC_GAMEPAD_DPAD_UPRIGHT,
		BC_GAMEPAD_DPAD_DOWNLEFT,
		BC_GAMEPAD_DPAD_DOWNRIGHT,
		BC_NumKeys = BC_MEDIASELECT - BC_UNASSIGNED + 1, // IMPORTANT: Make sure to update these if you modify the values above
		BC_NumMouse = BC_MOUSE_BTN32 - BC_MOUSE_LEFT + 1,
		BC_NumGamepad = BC_GAMEPAD_DPAD_DOWNRIGHT - BC_GAMEPAD_A + 1,
		BC_Count = BC_NumKeys + BC_NumMouse + BC_NumGamepad,
	};

	/**	Contains data about a button input event. */
	struct ButtonEvent
	{
	public:
		ButtonEvent()
			:mIsUsed(false)
		{ }

		ButtonCode buttonCode; /**< Button code this event is referring to. */
		UINT64 timestamp; /**< Timestamp in ticks when the event happened. */
		UINT32 deviceIdx; /**< Index of the device that the event originated from. */

		/**	Query is the pressed button a keyboard button. */
		bool isKeyboard() const { return (buttonCode & 0xC0000000) == 0; }

		/** Query is the pressed button a mouse button. */
		bool isMouse() const { return (buttonCode & 0x80000000) != 0; }

		/** Query is the pressed button a gamepad button. */
		bool isGamepad() const { return (buttonCode & 0x40000000) != 0; }

		/**
		 * Check if the event has been marked as used. Internally this means nothing but caller might choose to ignore an 
		 * used event.
		 */
		bool isUsed() const { return mIsUsed; }

		/** Mark the event as used. Internally this means nothing but caller might choose to ignore an used event. */
		void markAsUsed() const { mIsUsed = true; }
	private:
		mutable bool mIsUsed;
	};

	/**
	 * Pointer buttons. Generally these correspond to mouse buttons, but may be used in some form for touch input as well.
	 */
	enum class PointerEventButton
	{
		Left, Middle, Right, Count
	};

	/**	Type of pointer event.*/
	enum class PointerEventType
	{
		CursorMoved,
		ButtonPressed,
		ButtonReleased,
		DoubleClick
	};

	/**
	 * Event that gets sent out when user interacts with the screen in some way, usually by moving the mouse cursor or
	 * using touch input.
	 */
	struct PointerEvent
	{
	public:
		PointerEvent()
			: button(PointerEventButton::Left), type(PointerEventType::CursorMoved), shift(false)
			, control(false), alt(false), mouseWheelScrollAmount(0.0f), mIsUsed(false)
		{
			buttonStates[0] = false;
			buttonStates[1] = false;
			buttonStates[2] = false;
		}

		Vector2I screenPos; /**< Screen position where the input event occurred. */
		Vector2I delta; /**< Change in movement since last sent event. */
		/** States of the pointer buttons (for example mouse buttons). */
		bool buttonStates[(UINT32)PointerEventButton::Count]; 
		/**
		 * Button that triggered the pointer event. Might be irrelevant depending on event type. (for example move events
		 * don't correspond to a button.
		 */
		PointerEventButton button;
		PointerEventType type; /**< Type of the pointer event. */

		bool shift; /**< Is shift button on the keyboard being held down. */
		bool control; /**< Is control button on the keyboard being held down. */
		bool alt; /**< Is alt button on the keyboard being held down. */

		/** If mouse wheel is being scrolled, what is the amount. Only relevant for move events. */
		float mouseWheelScrollAmount; 

		/**
		 * Check if the event has been marked as used. Internally this means nothing but caller might choose to ignore an 
		 * used event.
		 */
		bool isUsed() const { return mIsUsed; }

		/** Mark the event as used. Internally this means nothing but caller might choose to ignore an used event. */
		void markAsUsed() const { mIsUsed = true; }

	private:
		mutable bool mIsUsed;
	};

	/**	Types of special input commands. */
	enum class InputCommandType
	{
		CursorMoveLeft, CursorMoveRight, CursorMoveUp, CursorMoveDown, 
		SelectLeft, SelectRight, SelectUp, SelectDown,
		Escape, Delete, Backspace, Return, Confirm
	};

	/**
	 * Event that gets sent out when user inputs some text. These events may be preceeded by normal button events if user 
	 * is typing on a keyboard. 
	 */
	struct TextInputEvent
	{
	public:
		TextInputEvent()
			:mIsUsed(false)
		{ }

		UINT32 textChar; /**< Character the that was input. */

		/**
		 * Check if the event has been marked as used. Internally this means nothing but caller might choose to ignore an
		 * used event.
		 */
		bool isUsed() const { return mIsUsed; }

		/** Mark the event as used. Internally this means nothing but caller might choose to ignore an used event. */
		void markAsUsed() const { mIsUsed = true; }

	private:
		mutable bool mIsUsed;
	};

	/**	Types of input devices. */
	enum class InputDevice
	{
		Keyboard,
		Mouse,
		Gamepad,
		Count // Keep at end
	};

	/**	Common input axis types. */
	enum class InputAxis
	{
		MouseX, /**< Mouse axis X. Provides unnormalized relative movement. */
		MouseY, /**< Mouse axis Y. Provides unnormalized relative movement. */
		MouseZ, /**< Mouse wheel/scroll axis. Provides unnormalized relative movement. */
		LeftStickX, /**< Gamepad left stick X. Provides normalized ([-1, 1] range) absolute position. */
		LeftStickY, /**<  Gamepad left stick Y. Provides normalized ([-1, 1] range) absolute position. */
		RightStickX, /**< Gamepad right stick X. Provides normalized ([-1, 1] range) absolute position.*/
		RightStickY, /**< Gamepad right stick Y. Provides normalized ([-1, 1] range) absolute position. */
		LeftTrigger, /**< Gamepad left trigger. Provides normalized ([-1, 1] range) absolute position. */
		RightTrigger, /**< Gamepad right trigger. Provides normalized ([-1, 1] range) absolute position. */
		Count // Keep at end
	};

	/**	Modifiers used with along with keyboard buttons. */
	enum class ButtonModifier
	{
		None = 0x00,
		Shift = 0x01,
		Ctrl = 0x02,
		Alt = 0x04,
		ShiftCtrl = 0x03,
		CtrlAlt = 0x06,
		ShiftAlt = 0x05,
		ShiftCtrlAlt = 0x07
	};

	/** @} */
}
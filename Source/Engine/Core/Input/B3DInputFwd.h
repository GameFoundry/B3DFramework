//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
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
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Input)) ButtonCode : u32
	{
		Unassigned = 0x00,
		Escape = 0x01,
		Key1 = 0x02,
		Key2 = 0x03,
		Key3 = 0x04,
		Key4 = 0x05,
		Key5 = 0x06,
		Key6 = 0x07,
		Key7 = 0x08,
		Key8 = 0x09,
		Key9 = 0x0A,
		Key0 = 0x0B,
		Minus = 0x0C, // - on main keyboard
		Equals = 0x0D,
		Backspace = 0x0E, // backspace
		Tab = 0x0F,
		Q = 0x10,
		W = 0x11,
		E = 0x12,
		R = 0x13,
		T = 0x14,
		Y = 0x15,
		U = 0x16,
		I = 0x17,
		O = 0x18,
		P = 0x19,
		LeftBracket = 0x1A,
		RightBracket = 0x1B,
		Enter = 0x1C, // Enter on main keyboard
		LeftControl = 0x1D,
		A = 0x1E,
		S = 0x1F,
		D = 0x20,
		F = 0x21,
		G = 0x22,
		H = 0x23,
		J = 0x24,
		K = 0x25,
		L = 0x26,
		Semicolon = 0x27,
		Apostrophe = 0x28,
		Grave = 0x29, // accent
		LeftShift = 0x2A,
		Backslash = 0x2B,
		Z = 0x2C,
		X = 0x2D,
		C = 0x2E,
		V = 0x2F,
		B = 0x30,
		N = 0x31,
		M = 0x32,
		Comma = 0x33,
		Period = 0x34, // . on main keyboard
		Slash = 0x35, // / on main keyboard
		RightShift = 0x36,
		NumpadMultiply = 0x37, // * on numeric keypad
		LeftAlt = 0x38, // left Alt
		Space = 0x39,
		CapsLock = 0x3A,
		F1 = 0x3B,
		F2 = 0x3C,
		F3 = 0x3D,
		F4 = 0x3E,
		F5 = 0x3F,
		F6 = 0x40,
		F7 = 0x41,
		F8 = 0x42,
		F9 = 0x43,
		F10 = 0x44,
		NumLock = 0x45,
		ScrollLock = 0x46, // Scroll Lock
		Numpad7 = 0x47,
		Numpad8 = 0x48,
		Numpad9 = 0x49,
		NumpadMinus = 0x4A, // - on numeric keypad
		Numpad4 = 0x4B,
		Numpad5 = 0x4C,
		Numpad6 = 0x4D,
		NumpadPlus = 0x4E, // + on numeric keypad
		Numpad1 = 0x4F,
		Numpad2 = 0x50,
		Numpad3 = 0x51,
		Numpad0 = 0x52,
		NumpadDecimal = 0x53, // . on numeric keypad
		OEM102 = 0x56, // < > | on UK/Germany keyboards
		F11 = 0x57,
		F12 = 0x58,
		F13 = 0x64, //                     (NEC PC98)
		F14 = 0x65, //                     (NEC PC98)
		F15 = 0x66, //                     (NEC PC98)
		Kana = 0x70, // (Japanese keyboard)
		ABNTC1 = 0x73, // / ? on Portugese (Brazilian) keyboards
		Convert = 0x79, // (Japanese keyboard)
		NoConvert = 0x7B, // (Japanese keyboard)
		Yen = 0x7D, // (Japanese keyboard)
		ABNTC2 = 0x7E, // Numpad . on Portugese (Brazilian) keyboards
		NumadEquals = 0x8D, // = on numeric keypad (NEC PC98)
		PreviousTrack = 0x90, // Previous Track (BC_CIRCUMFLEX on Japanese keyboard)
		At = 0x91, //                     (NEC PC98)
		Colon = 0x92, //                     (NEC PC98)
		Underline = 0x93, //                     (NEC PC98)
		Kanji = 0x94, // (Japanese keyboard)
		Stop = 0x95, //                     (NEC PC98)
		AX = 0x96, //                     (Japan AX)
		Unlabeled = 0x97, //                        (J3100)
		NextTrack = 0x99, // Next Track
		NumpadEnter = 0x9C, // Enter on numeric keypad
		RightControl = 0x9D,
		Mute = 0xA0, // Mute
		Calculator = 0xA1, // Calculator
		PlayPause = 0xA2, // Play / Pause
		MediaStop = 0xA4, // Media Stop
		VolumeDown = 0xAE, // Volume -
		VolumeUp = 0xB0, // Volume +
		WebHome = 0xB2, // Web home
		NumpadComma = 0xB3, // , on numeric keypad (NEC PC98)
		NumpadDivide = 0xB5, // / on numeric keypad
		SysRq = 0xB7,
		RightAlt = 0xB8, // right Alt
		Pause = 0xC5, // Pause
		Home = 0xC7, // Home on arrow keypad
		ArrowUp = 0xC8, // UpArrow on arrow keypad
		PageUp = 0xC9, // PgUp on arrow keypad
		ArrowLeft = 0xCB, // LeftArrow on arrow keypad
		ArrowRight = 0xCD, // RightArrow on arrow keypad
		End = 0xCF, // End on arrow keypad
		ArrowDown = 0xD0, // DownArrow on arrow keypad
		PageDown = 0xD1, // PgDn on arrow keypad
		Insert = 0xD2, // Insert on arrow keypad
		Delete = 0xD3, // Delete on arrow keypad
		LeftWindows = 0xDB, // Left Windows key
		RightWindows = 0xDC, // Right Windows key
		ApplicationMenu = 0xDD, // AppMenu key
		Power = 0xDE, // System Power
		Sleep = 0xDF, // System Sleep
		Awake = 0xE3, // System Wake
		WebSearch = 0xE5, // Web Search
		WebFavorites = 0xE6, // Web Favorites
		WebRefresh = 0xE7, // Web Refresh
		WebStop = 0xE8, // Web Stop
		WebForward = 0xE9, // Web Forward
		WebBack = 0xEA, // Web Back
		MyComputer = 0xEB, // My Computer
		Mail = 0xEC, // Mail
		MediaSelect = 0xED, // Media Select
		MouseLeft = 0x800000EE, // Mouse buttons - Most important bit signifies this key is a mouse button
		MouseRight,
		MouseMiddle,
		MouseButton4,
		MouseButton5,
		MouseButton6,
		MouseButton7,
		MouseButton8,
		MouseButton9,
		MouseButton10,
		MouseButton11,
		MouseButton12,
		MouseButton13,
		MouseButton14,
		MouseButton15,
		MouseButton16,
		MouseButton17,
		MouseButton18,
		MouseButton19,
		MouseButton20,
		MouseButton21,
		MouseButton22,
		MouseButton23,
		MouseButton24,
		MouseButton25,
		MouseButton26,
		MouseButton27,
		MouseButton28,
		MouseButton29,
		MouseButton30,
		MouseButton31,
		MouseButton32,
		GamepadA = 0x4000010F, // Joystick/Gamepad buttons- Second most important bit signifies key is a gamepad button
		GamepadB, // Similar to keyboard names, these are for convenience named after Xbox controller buttons
		GamepadX, // but if some other controller is connected you will need to learn yourself which of these
		GamepadY, // corresponds to which actual button on the controller.
		GamepadLeftBumper,
		GamepadRightBumper,
		GamepadLeftStick,
		GamepadRightStick,
		GamepadBack,
		GamepadStart,
		GamepadDPadLeft,
		GamepadDPatRight,
		GamepadDPadUp,
		GamepadDPadDown,
		GamepadButton1,
		GamepadButton2,
		GamepadButton3,
		GamepadButton4,
		GamepadButton5,
		GamepadButton6,
		GamepadButton7,
		GamepadButton8,
		GamepadButton9,
		GamepadButton10,
		GamepadButton11,
		GamepadButton12,
		GamepadButton13,
		GamepadButton14,
		GamepadButton15,
		GamepadButton16,
		GamepadButton17,
		GamepadButton18,
		GamepadButton19,
		GamepadButton20,
		GamepadDPadUpLeft,
		GamepadDPadUpRight,
		GamepadDPadDownLeft,
		GamepadDPadDownRight,
		KeyboardKeyCount = MediaSelect - Unassigned + 1, // IMPORTANT: Make sure to update these if you modify the values above
		MouseKeyCount = MouseButton32 - MouseLeft + 1,
		GamepadKeyCount = GamepadDPadDownRight - GamepadA + 1,
		// +1 covers the unused index between the mouse and gamepad ranges, so the total spans the highest masked index
		TotalKeyCount = KeyboardKeyCount + MouseKeyCount + GamepadKeyCount + 1,
	};

	/**	Contains data about a button input event. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Input)) ButtonEvent
	{
		ButtonCode ButtonCode; /**< Button code this event is referring to. */
		u64 Timestamp; /**< Timestamp in ticks when the event happened. */
		u32 DeviceIndex; /**< Index of the device that the event originated from. */
		mutable bool IsUsed = false; /**< This will be set to true if some previous event receiver has marked the event as used. */

		/**	Query is the pressed button a keyboard button. */
		bool IsKeyboard() const { return ((u32)ButtonCode & 0xC0000000) == 0; }

		/** Query is the pressed button a mouse button. */
		bool IsMouse() const { return ((u32)ButtonCode & 0x80000000) != 0; }

		/** Query is the pressed button a gamepad button. */
		bool IsGamepad() const { return ((u32)ButtonCode & 0x40000000) != 0; }
	};

	/**
	 * Pointer buttons. Generally these correspond to mouse buttons, but may be used in some form for touch input as well.
	 */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Input)) PointerEventButton
	{
		Left,
		Middle,
		Right,
		Count
	};

	/**	Type of pointer event.*/
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Input)) PointerEventType
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
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Input)) PointerEvent
	{
		Vector2I ScreenPos = Vector2I::kZero; /**< Screen position where the input event occurred. */
		Vector2I Delta = Vector2I::kZero; /**< Change in movement since last sent event. */

		B3D_SCRIPT_EXPORT(Exclude(true))
		bool ButtonStates[(u32)PointerEventButton::Count] = { false, false, false }; /**< States of the pointer buttons (for example mouse buttons). */

		/**
		 * Button that triggered the pointer event. Might be irrelevant depending on event type. (for example move events
		 * don't correspond to a button.
		 */
		PointerEventButton Button = PointerEventButton::Left;
		PointerEventType Type = PointerEventType::CursorMoved; /**< Type of the pointer event. */

		bool Shift = false; /**< Is shift button on the keyboard being held down. */
		bool Control = false; /**< Is control button on the keyboard being held down. */
		bool Alt = false; /**< Is alt button on the keyboard being held down. */

		float MouseWheelScrollAmount = 0.0f; /**< If mouse wheel is being scrolled, what is the amount. Only relevant for move events. */

		mutable bool IsUsed = false; /**< This will be set to true if some previous event receiver has marked the event as used. */
	};

	/**	Types of special input commands. */
	enum class InputCommandType
	{
		CursorMoveLeft,
		CursorMoveRight,
		CursorMoveUp,
		CursorMoveDown,
		SelectLeft,
		SelectRight,
		SelectUp,
		SelectDown,
		Escape,
		Delete,
		Backspace,
		Return,
		Confirm,
		Tab
	};

	/**
	 * Event that gets sent out when user inputs some text. These events may be preceeded by normal button events if user
	 * is typing on a keyboard.
	 */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Input)) TextInputEvent
	{
		u32 TextChar; /**< Character the that was input. */
		mutable bool IsUsed = false; /**< This will be set to true if some previous event receiver has marked the event as used. */
	};

	/**	Contains data about a gamepad connection or disconnection event. */
	struct GamepadConnectionEvent
	{
		u32 DeviceIndex; /**< Index of the gamepad device. Same index is used when querying button and axis state. */
		String Name; /**< Name of the gamepad device. */
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
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Input)) InputAxis
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
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Input)) ButtonModifier
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
} // namespace b3d

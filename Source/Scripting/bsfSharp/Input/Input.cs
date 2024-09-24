//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.Runtime.CompilerServices;

namespace bs
{
    /** @addtogroup Input
     *  @{
     */

    /// <summary>
    /// Contains data about a button input event.
    /// </summary>
	public struct ButtonEvent
	{
		internal ButtonCode buttonCode;
        internal int deviceIdx;
        internal bool isUsed;

        /// <summary>
        /// Creates a new button input event. For runtime use only.
        /// </summary>
        /// <param name="buttonCode">Button code this event is referring to.</param>
        /// <param name="deviceIdx">Index of the device that the event originated from.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
	    internal ButtonEvent(ButtonCode buttonCode, int deviceIdx, bool isUsed)
	    {
	        this.buttonCode = buttonCode;
	        this.deviceIdx = deviceIdx;
            this.isUsed = isUsed;
	    }

        /// <summary>
        /// Button code this event is referring to.
        /// </summary>
        public ButtonCode Button { get { return buttonCode; } }

        /// <summary>
        /// Index of the device that the event originated from.
        /// </summary>
        public int DeviceIndex { get { return deviceIdx; } }

        /// <summary>
        /// Query is the pressed button a keyboard button.
        /// </summary>
		public bool IsKeyboard { get { return ((int)buttonCode & 0xC0000000) == 0; }}

        /// <summary>
        /// Query is the pressed button a mouse button.
        /// </summary>
        public bool IsMouse { get { return ((int)buttonCode & 0x80000000) != 0; } }

        /// <summary>
        /// Query is the pressed button a gamepad button.
        /// </summary>
        public bool IsGamepad { get { return ((int)buttonCode & 0x40000000) != 0; } }

        /// <summary>
        /// Returns true if the event was handled previously by some internal system (like GUI).
        /// </summary>
        public bool IsUsed { get { return isUsed; } }
    };

    /// <summary>
    /// Pointer buttons. Generally these correspond to mouse buttons, but may be used in some form for touch input as well.
    /// </summary>
    public enum PointerButton // Note: Must match C++ enum PointerEventButton
	{
		Left, Middle, Right, Count
	};

    /// <summary>
    /// Event that gets sent out when user interacts with the screen in some way, usually by moving the mouse cursor or
    /// using touch input.
    /// </summary>
    public struct PointerEvent
    {
        internal Vector2I screenPos;
        internal Vector2I delta;
        internal PointerButton button;

        internal bool shift;
        internal bool control;
        internal bool alt;

        internal float mouseWheelScrollAmount;
        internal bool isUsed;

        /// <summary>
        /// Creates a new pointer event. For runtime use only.
        /// </summary>
        /// <param name="screenPos">Screen position where the input event occurred.</param>
        /// <param name="delta">Change in movement since last sent event.</param>
        /// <param name="button">Button that triggered the pointer event. Might be irrelevant depending on event type.
        ///                      (for example move events don't correspond to a button.</param>
        /// <param name="shift">Is shift button on the keyboard being held down.</param>
        /// <param name="control">Is control button on the keyboard being held down.</param>
        /// <param name="alt">Is alt button on the keyboard being held down.</param>
        /// <param name="mouseWheelScrollAmount">If mouse wheel is being scrolled, what is the amount. Only relevant for
        ///                                      move events.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        internal PointerEvent(Vector2I screenPos, Vector2I delta, PointerButton button,
            bool shift, bool control, bool alt, float mouseWheelScrollAmount, bool isUsed)
        {
            this.screenPos = screenPos;
            this.delta = delta;
            this.button = button;

            this.shift = shift;
            this.control = control;
            this.alt = alt;

            this.mouseWheelScrollAmount = mouseWheelScrollAmount;
            this.isUsed = isUsed;
        }

        /// <summary>
        /// Screen position where the input event occurred.
        /// </summary>
        public Vector2I ScreenPos { get { return screenPos; } }

        /// <summary>
        /// Change in movement since last sent event.
        /// </summary>
        public Vector2I Delta { get { return delta; } }

        /// <summary>
        /// Button that triggered the pointer event. Might be irrelevant depending on event type.
        /// (for example move events don't correspond to a button.
        /// </summary>
        public PointerButton Button { get { return button; } }

        /// <summary>
        /// Is shift button on the keyboard being held down.
        /// </summary>
        public bool Shift { get { return shift; } }

        /// <summary>
        /// Is control button on the keyboard being held down.
        /// </summary>
        public bool Control { get { return control; } }

        /// <summary>
        /// Is alt button on the keyboard being held down.
        /// </summary>
        public bool Alt { get { return alt; } }

        /// <summary>
        /// If mouse wheel is being scrolled, what is the amount. Only relevant for move events.
        /// </summary>
        public float ScrollAmount { get { return mouseWheelScrollAmount; } }

        /// <summary>
        /// Returns true if the event was handled previously by some internal system (like GUI).
        /// </summary>
        public bool IsUsed { get { return isUsed; } }
    }

    /// <summary>
    /// Event that gets sent out when user inputs some text. These events may be preceeded by normal button events if user
    /// is typing on a keyboard.
    /// </summary>
    public struct TextInputEvent
    {
        internal int textChar;
        internal bool isUsed;

        /// <summary>
        /// Creates a new text input event. For runtime use only.
        /// </summary>
        /// <param name="textChar">Character the that was input.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        internal TextInputEvent(int textChar, bool isUsed)
        {
            this.textChar = textChar;
            this.isUsed = isUsed;
        }

        /// <summary>
        /// Character the that was input.
        /// </summary>
        public int Char { get { return textChar; } }

        /// <summary>
        /// Returns true if the event was handled previously by some internal system (like GUI).
        /// </summary>
        public bool IsUsed { get { return isUsed; } }
    }

    /// <summary>
    /// Allows you to query and receive events from all connected input devices.
    /// </summary>
    public static class Input
    {
        public delegate void ButtonEventDelegate(ButtonEvent ev);
        public delegate void TextInputEventDelegate(TextInputEvent ev);
        public delegate void PointerEventDelegate(PointerEvent ev);

        /// <summary>
        /// Triggered when a button on any device is pressed.
        /// </summary>
		public static event ButtonEventDelegate OnButtonDown;

        /// <summary>
        /// Triggered when a button on any device is released.
        /// </summary>
        public static event ButtonEventDelegate OnButtonUp;

        /// <summary>
        /// Triggered when a textual character is entered.
        /// </summary>
        public static event TextInputEventDelegate OnCharInput;

        /// <summary>
        /// Triggered when the pointing device (mouse, touch) is moved.
        /// </summary>
        public static event PointerEventDelegate OnPointerMoved;

        /// <summary>
        /// Triggered when a button on the pointing device (mouse, touch) is pressed.
        /// </summary>
        public static event PointerEventDelegate OnPointerPressed;

        /// <summary>
        /// Triggered when a button on the pointing device (mouse, touch) is released.
        /// </summary>
        public static event PointerEventDelegate OnPointerReleased;

        /// <summary>
        /// Triggered when a button on the pointing device (mouse, touch) is pressed twice in rapid succession.
        /// </summary>
        public static event PointerEventDelegate OnPointerDoubleClick;

        /// <summary>
        /// Returns value of the specified input axis.
        /// </summary>
        /// <param name="axis">Type of axis to query.</param>
        /// <param name="deviceIdx">Index of the device in case more than one is hooked up (0 - primary).</param>
        /// <returns>Value of the axis in range [-1.0, 1.0]. Canan be outside the range for devices with unbound axes
        ///          (for example mouse).</returns>
        public static float GetAxisValue(InputAxis axis, int deviceIdx = 0)
        {
            return Internal_GetAxisValue(axis, deviceIdx);
        }

        /// <summary>
        /// Query if the provided button is currently being held (true for every frame the button is held for).
        /// </summary>
        /// <param name="code">Code of the button to query.</param>
        /// <param name="deviceIdx">Device to query the button on (0 - primary).</param>
        /// <returns>True if the button is held.</returns>
        public static bool IsButtonHeld(ButtonCode code, int deviceIdx = 0)
        {
            return Internal_IsButtonHeld(code, deviceIdx);
        }

        /// <summary>
        /// Query if the provided button is currently being released (only true for one frame).
        /// </summary>
        /// <param name="code">Code of the button to query.</param>
        /// <param name="deviceIdx">Device to query the button on (0 - primary).</param>
        /// <returns>True if the button is being released.</returns>
        public static bool IsButtonUp(ButtonCode code, int deviceIdx = 0)
        {
            return Internal_IsButtonUp(code, deviceIdx);
        }

        /// <summary>
        /// Query if the provided button is currently being pressed (only true for one frame).
        /// </summary>
        /// <param name="code">Code of the button to query.</param>
        /// <param name="deviceIdx">Device to query the button on (0 - primary).</param>
        /// <returns>True if the button is being pressed.</returns>
        public static bool IsButtonDown(ButtonCode code, int deviceIdx = 0)
        {
            return Internal_IsButtonDown(code, deviceIdx);
        }

        /// <summary>
        /// Query if the provided pointer button is currently being held (true for every frame the button is held for).
        /// </summary>
        /// <param name="code">Code of the button to query.</param>
        /// <returns>True if the button is being held.</returns>
        public static bool IsPointerButtonHeld(PointerButton code)
        {
            return Internal_IsPointerButtonHeld(code);
        }

        /// <summary>
        /// Query if the provided pointer button is currently being being released (only true for one frame).
        /// </summary>
        /// <param name="code">Code of the button to query.</param>
        /// <returns>True if the button is being released.</returns>
        public static bool IsPointerButtonUp(PointerButton code)
        {
            return Internal_IsPointerButtonUp(code);
        }

        /// <summary>
        /// Query if the provided pointer button is currently being being pressed (only true for one frame).
        /// </summary>
        /// <param name="code">Code of the button to query.</param>
        /// <returns>True if the button is being pressed.</returns>
        public static bool IsPointerButtonDown(PointerButton code)
        {
            return Internal_IsPointerButtonDown(code);
        }

        /// <summary>
        /// Query has the left pointer button been double-clicked this frame.
        /// </summary>
        /// <returns>True if double-click occurred.</returns>
        public static bool IsPointerDoubleClicked()
        {
            return Internal_IsPointerDoubleClicked();
        }

        /// <summary>
        /// Returns position of the pointer (for example mouse cursor) relative to the screen.
        /// </summary>
        public static Vector2I PointerPosition
        {
            get
            {
                Vector2I value;
                Internal_GetPointerPosition(out value);
                return value;
            }
        }

        /// <summary>
        /// Returns difference pointer position of the current and last frame.
        /// </summary>
        public static Vector2I PointerDelta
        {
            get
            {
                Vector2I value;
                Internal_GetPointerDelta(out value);
                return value;
            }
        }

        /// <summary>
        /// Triggered by runtime when a button is pressed.
        /// </summary>
        /// <param name="code">Code of the pressed button.</param>
        /// <param name="deviceIdx">Device the event originated from.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        private static void Internal_TriggerButtonDown(ButtonCode code, int deviceIdx, bool isUsed)
        {
            ButtonEvent ev = new ButtonEvent(code, deviceIdx, isUsed);

            if (OnButtonDown != null)
                OnButtonDown(ev);
        }

        /// <summary>
        /// Triggered by runtime when a button is released.
        /// </summary>
        /// <param name="code">Code of the released button.</param>
        /// <param name="deviceIdx">Device the event originated from.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        private static void Internal_TriggerButtonUp(ButtonCode code, int deviceIdx, bool isUsed)
        {
            ButtonEvent ev = new ButtonEvent(code, deviceIdx, isUsed);

            if (OnButtonUp != null)
                OnButtonUp(ev);
        }

        /// <summary>
        /// Triggered by runtime when character is input.
        /// </summary>
        /// <param name="textChar">Code of input character.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        private static void Internal_TriggerCharInput(int textChar, bool isUsed)
        {
            TextInputEvent ev = new TextInputEvent(textChar, isUsed);

            if (OnCharInput != null)
                OnCharInput(ev);
        }

        /// <summary>
        /// Triggers when some pointing device (mouse cursor, touch) moves.
        /// </summary>
        /// <param name="screenPos">Screen position where the input event occurred.</param>
        /// <param name="delta">Change in movement since last sent event.</param>
        /// <param name="button">Button that triggered the pointer event. Might be irrelevant depending on event type.
        ///                      (for example move events don't correspond to a button.</param>
        /// <param name="shift">Is shift button on the keyboard being held down.</param>
        /// <param name="ctrl">Is control button on the keyboard being held down.</param>
        /// <param name="alt">Is alt button on the keyboard being held down.</param>
        /// <param name="scrollAmount">If mouse wheel is being scrolled, what is the amount. Only relevant for
        ///                            move events.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        private static void Internal_TriggerPointerMove(Vector2I screenPos, Vector2I delta, PointerButton button, bool shift,
            bool ctrl, bool alt, float scrollAmount, bool isUsed)
        {
            PointerEvent ev = new PointerEvent(screenPos, delta, button, shift, ctrl, alt, scrollAmount, isUsed);

            if (OnPointerMoved != null)
                OnPointerMoved(ev);
        }

        /// <summary>
        /// Triggers when some pointing device (mouse cursor, touch) button is pressed.
        /// </summary>
        /// <param name="screenPos">Screen position where the input event occurred.</param>
        /// <param name="delta">Change in movement since last sent event.</param>
        /// <param name="button">Button that triggered the pointer event. Might be irrelevant depending on event type.
        ///                      (for example move events don't correspond to a button.</param>
        /// <param name="shift">Is shift button on the keyboard being held down.</param>
        /// <param name="ctrl">Is control button on the keyboard being held down.</param>
        /// <param name="alt">Is alt button on the keyboard being held down.</param>
        /// <param name="scrollAmount">If mouse wheel is being scrolled, what is the amount. Only relevant for
        ///                            move events.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        private static void Internal_TriggerPointerPressed(Vector2I screenPos, Vector2I delta, PointerButton button, bool shift,
            bool ctrl, bool alt, float scrollAmount, bool isUsed)
        {
            PointerEvent ev = new PointerEvent(screenPos, delta, button, shift, ctrl, alt, scrollAmount, isUsed);

            if (OnPointerPressed != null)
                OnPointerPressed(ev);
        }

        /// <summary>
        /// Triggers when some pointing device (mouse cursor, touch) button is released.
        /// </summary>
        /// <param name="screenPos">Screen position where the input event occurred.</param>
        /// <param name="delta">Change in movement since last sent event.</param>
        /// <param name="button">Button that triggered the pointer event. Might be irrelevant depending on event type.
        ///                      (for example move events don't correspond to a button.</param>
        /// <param name="shift">Is shift button on the keyboard being held down.</param>
        /// <param name="ctrl">Is control button on the keyboard being held down.</param>
        /// <param name="alt">Is alt button on the keyboard being held down.</param>
        /// <param name="scrollAmount">If mouse wheel is being scrolled, what is the amount. Only relevant for
        ///                            move events.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        private static void Internal_TriggerPointerReleased(Vector2I screenPos, Vector2I delta, PointerButton button, bool shift,
            bool ctrl, bool alt, float scrollAmount, bool isUsed)
        {
            PointerEvent ev = new PointerEvent(screenPos, delta, button, shift, ctrl, alt, scrollAmount, isUsed);

            if (OnPointerReleased != null)
                OnPointerReleased(ev);
        }

        /// <summary>
        /// Triggers when some pointing device (mouse cursor, touch) button is double clicked.
        /// </summary>
        /// <param name="screenPos">Screen position where the input event occurred.</param>
        /// <param name="delta">Change in movement since last sent event.</param>
        /// <param name="button">Button that triggered the pointer event. Might be irrelevant depending on event type.
        ///                      (for example move events don't correspond to a button.</param>
        /// <param name="shift">Is shift button on the keyboard being held down.</param>
        /// <param name="ctrl">Is control button on the keyboard being held down.</param>
        /// <param name="alt">Is alt button on the keyboard being held down.</param>
        /// <param name="scrollAmount">If mouse wheel is being scrolled, what is the amount. Only relevant for
        ///                            move events.</param>
        /// <param name="isUsed">Set to true if the event was handled previously by some internal system (like GUI).</param>
        private static void Internal_TriggerPointerDoubleClick(Vector2I screenPos, Vector2I delta, PointerButton button, bool shift,
            bool ctrl, bool alt, float scrollAmount, bool isUsed)
        {
            PointerEvent ev = new PointerEvent(screenPos, delta, button, shift, ctrl, alt, scrollAmount, isUsed);

            if (OnPointerDoubleClick != null)
                OnPointerDoubleClick(ev);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetAxisValue(InputAxis axis, int deviceIdx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsButtonHeld(ButtonCode keyCode, int deviceIdx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsButtonUp(ButtonCode keyCode, int deviceIdx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsButtonDown(ButtonCode keyCode, int deviceIdx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsPointerDoubleClicked();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsPointerButtonHeld(PointerButton keyCode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsPointerButtonUp(PointerButton keyCode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsPointerButtonDown(PointerButton keyCode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetPointerPosition(out Vector2I position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetPointerDelta(out Vector2I delta);
    }

    /** @} */
}

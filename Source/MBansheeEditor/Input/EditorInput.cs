//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Input-Editor 
     *  @{
     */

    /// <summary>
    /// Companion class to <see cref="Input"/> for use in editor only. Supplies events that trigger regardless whether
    /// game is playing or not (unlike <see cref="Input"/>) which makes them usable for editor only scripts. Pollable
    /// input should still be used from <see cref="Input"/>.
    /// </summary>
    public static class EditorInput
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
        /// Triggered when a button on the pointing device (mouse, touch) is pressed twice in rappid succession.
        /// </summary>
        public static event PointerEventDelegate OnPointerDoubleClick;

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
    }

    /** @} */
}

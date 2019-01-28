//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup GUI_Engine
     *  @{
     */

    /// <summary>
    /// GUI element that accepts Unicode textual input. It can be single or multi-line and handles various types of text 
    /// manipulation.
    /// </summary>
    public sealed class GUITextBox : GUIElement
    {
        /// <summary>
        /// Triggered whenever input text has changed.
        /// </summary>
        public event Action<string> OnChanged;

        /// <summary>
        /// Triggered whenever user confirms input.
        /// </summary>
        public event Action OnConfirmed;

        /// <summary>
        /// Creates a new text box element.
        /// </summary>
        /// <param name="multiline">Determines should the input box allow text that spans multiple lines.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUITextBox(bool multiline, string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, multiline, style, options);
        }

        /// <summary>
        /// Creates a new text box element.
        /// </summary>
        /// <param name="multiline">Determines should the input box allow text that spans multiple lines.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUITextBox(bool multiline, params GUIOption[] options)
        {
            Internal_CreateInstance(this, multiline, "", options);
        }

        /// <summary>
        /// Creates a new single-line text box element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUITextBox(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, false, style, options);
        }

        /// <summary>
        /// Creates a new single-line text box element.
        /// </summary>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUITextBox(params GUIOption[] options)
        {
            Internal_CreateInstance(this, false, "", options);
        }

        /// <summary>
        /// Text currently entered in the text box.
        /// </summary>
        public string Text
        {
            get { string value; Internal_GetText(mCachedPtr, out value); return value; }
            set { Internal_SetText(mCachedPtr, value); }
        }

        /// <summary>
        /// Colors the element with a specific tint.
        /// </summary>
        /// <param name="color">Tint to apply to the element.</param>
        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, ref color);
        }

        /// <summary>
        /// Triggered by the native interop object when the text box value is changed.
        /// </summary>
        /// <param name="newValue">New value in the text box.</param>
        private void Internal_DoOnChanged(string newValue)
        {
            if (OnChanged != null)
                OnChanged(newValue);
        }

        /// <summary>
        /// Triggered by the native interop object when the user confirms the input.
        /// </summary>
        private void Internal_DoOnConfirmed()
        {
            if (OnConfirmed != null)
                OnConfirmed();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUITextBox instance, bool multiline, string style, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetText(IntPtr nativeInstance, string text);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetText(IntPtr nativeInstance, out string text);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}

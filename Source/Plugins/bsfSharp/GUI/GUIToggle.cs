//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup GUI_Engine
     *  @{
     */

    /// <summary>
    /// GUI element representing a toggle (on/off) button.
    /// </summary>
    public sealed class GUIToggle : GUIElement
    {
        public delegate void OnClickDelegate();
        public delegate void OnHoverDelegate();
        public delegate void OnOutDelegate();
        public delegate void OnToggleDelegate(bool toggled);
        public delegate void OnDoubleClickDelegate();

        public event OnClickDelegate OnClick;
        public event OnHoverDelegate OnHover;
        public event OnOutDelegate OnOut;
        public event OnToggleDelegate OnToggled;
        public event OnDoubleClickDelegate OnDoubleClick;

        /// <summary>
        /// On/off state of the button.
        /// </summary>
        public bool Value
        {
            get { return Internal_GetValue(mCachedPtr); }
            set { Internal_SetValue(mCachedPtr, value); }
        }

        /// <summary>
        /// Creates a new toggle button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="toggleGroup">Optional toggle group that is used for grouping multiple toggle buttons 
        ///                           together. </param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIToggle(GUIContent content, GUIToggleGroup toggleGroup, string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ref content, toggleGroup, style, options);
        }

        /// <summary>
        /// Creates a new toggle button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIToggle(GUIContent content, string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ref content, null, style, options);
        }

        /// <summary>
        /// Creates a new toggle button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUIToggle(GUIContent content, string style)
        {
            Internal_CreateInstance(this, ref content, null, style, new GUIOption[0]);
        }

        /// <summary>
        /// Creates a new toggle button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIToggle(GUIContent content, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ref content, null, "", options);
        }

        /// <summary>
        /// Creates a new toggle button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="toggleGroup">Optional toggle group that is used for grouping multiple toggle buttons 
        ///                           together.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUIToggle(GUIContent content, GUIToggleGroup toggleGroup, string style)
        {
            Internal_CreateInstance(this, ref content, toggleGroup, style, new GUIOption[0]);
        }

        /// <summary>
        /// Creates a new toggle button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="toggleGroup">Optional toggle group that is used for grouping multiple toggle buttons 
        ///                           together.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIToggle(GUIContent content, GUIToggleGroup toggleGroup, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ref content, toggleGroup, "", options);
        }

        /// <summary>
        /// Updates the contents displayed on the button.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        public void SetContent(GUIContent content)
        {
            Internal_SetContent(mCachedPtr, ref content);
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
        /// Triggered by the native interop object when a click occurs.
        /// </summary>
        private void DoOnClick()
        {
            if (OnClick != null)
                OnClick();
        }

        /// <summary>
        /// Triggered by the native interop object when the pointer is hovered over the element.
        /// </summary>
        private void DoOnHover()
        {
            if (OnHover != null)
                OnHover();
        }

        /// <summary>
        /// Triggered by the native interop object when the pointer leaves the element.
        /// </summary>
        private void DoOnOut()
        {
            if (OnOut != null)
                OnOut();
        }

        /// <summary>
        /// Triggered by the native interop object when the button is toggle on or off.
        /// </summary>
        /// <param name="toggled">New state of the element.</param>
        private void DoOnToggled(bool toggled)
        {
            if (OnToggled != null)
                OnToggled(toggled);
        }

        /// <summary>
        /// Triggered by the native interop object when a double click occurs.
        /// </summary>
        private void DoOnDoubleClick()
        {
            if (OnDoubleClick != null)
                OnDoubleClick();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIToggle instance, ref GUIContent content,
            GUIToggleGroup toggleGroup, string style, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetContent(IntPtr nativeInstance, ref GUIContent content);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetValue(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetValue(IntPtr nativeInstance, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}

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
    /// GUI button that can be clicked. Has normal, hover and active states	with an optional label.
    /// </summary>
    public sealed class GUIButton : GUIElement
    {
        public delegate void OnClickDelegate();
        public delegate void OnDoubleClickDelegate();
        public delegate void OnHoverDelegate();
        public delegate void OnOutDelegate();

        /// <summary>
        /// Triggered when button is clicked.
        /// </summary>
        public event OnClickDelegate OnClick;

        /// <summary>
        /// Triggered when button is clicked twice in rapid succession.
        /// </summary>
        public event OnDoubleClickDelegate OnDoubleClick;

        /// <summary>
        /// Triggered when pointer hovers over the button.
        /// </summary>
        public event OnHoverDelegate OnHover;

        /// <summary>
        /// Triggered when pointer that was previously hovering leaves the button.
        /// </summary>
        public event OnOutDelegate OnOut;

        /// <summary>
        /// Creates a new button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIButton(GUIContent content, string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ref content, style, options);
        }

        /// <summary>
        /// Creates a new button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUIButton(GUIContent content, string style)
        {
            Internal_CreateInstance(this, ref content, style, new GUIOption[0]);
        }

        /// <summary>
        /// Creates a new button with the specified label.
        /// </summary>
        /// <param name="content">Content to display on the button.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIButton(GUIContent content, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ref content, "", options);
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
        /// Triggered by the native interop object when a double click occurs.
        /// </summary>
        private void DoOnDoubleClick()
        {
            if (OnDoubleClick != null)
                OnDoubleClick();
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

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIButton instance, ref GUIContent content, string style, 
            GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetContent(IntPtr nativeInstance, ref GUIContent content);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}

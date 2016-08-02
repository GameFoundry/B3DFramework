//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup GUI-Engine
     *  @{
     */

    /// <summary>
    /// GUI element representing an element with a draggable handle of a variable size.
    /// </summary>
    public class GUIScrollBar : GUIElement
    {
        /// <summary>
        /// Position of the scroll handle in percent (ranging [0, 1]).
        /// </summary>
        public float Position
        {
            get { return Internal_GetPosition(mCachedPtr); }
            set { Internal_SetPosition(mCachedPtr, value); }
        }

        /// <summary>
        /// Size of the scroll handle in percent (ranging [0, 1]) of the total scroll bar area.
        /// </summary>
        public float HandleSize
        {
            get { return Internal_GetHandleSize(mCachedPtr); }
            set { Internal_SetHandleSize(mCachedPtr, value); }
        }
        
        /// <summary>
        /// Colors the element with a specific tint.
        /// </summary>
        /// <param name="color">Tint to apply to the element.</param>
        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, ref color);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetPosition(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPosition(IntPtr nativeInstance, float percent);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetHandleSize(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetHandleSize(IntPtr nativeInstance, float percent);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /// <summary>
    /// GUI element representing a horizontal scroll bar.
    /// </summary>
    public sealed class GUIScrollBarH : GUIScrollBar
    {
        /// <summary>
        /// Event triggered whenever the user moves the scroll bar. Value reported is the position of the scroll bar handle
        /// in percent, in [0, 1] range.
        /// </summary>
        public event Action<float> OnScroll;

        /// <summary>
        /// Creates a new horizontal scroll bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIScrollBarH(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, style, options);
        }

        /// <summary>
        /// Creates a new horizontal scroll bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUIScrollBarH(string style = "")
        {
            Internal_CreateInstance(this, style, new GUIOption[0]);
        }

        private void Internal_DoOnScroll(float position)
        {
            if (OnScroll != null)
                OnScroll(position);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIScrollBarH instance, string style, GUIOption[] options);
    }

    /// <summary>
    /// GUI element representing a vertical scroll bar.
    /// </summary>
    public sealed class GUIScrollBarV : GUIScrollBar
    {
        /// <summary>
        /// Event triggered whenever the user moves the scroll bar. Value reported is the position of the scroll bar handle
        /// in percent, in [0, 1] range.
        /// </summary>
        public event Action<float> OnScroll;

        /// <summary>
        /// Creates a new vertical scroll bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIScrollBarV(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, style, options);
        }

        /// <summary>
        /// Creates a new vertical scroll bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUIScrollBarV(string style = "")
        {
            Internal_CreateInstance(this, style, new GUIOption[0]);
        }

        private void Internal_DoOnScroll(float position)
        {
            if (OnScroll != null)
                OnScroll(position);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIScrollBarV instance, string style, GUIOption[] options);
    }

    /// <summary>
    /// GUI element representing a horizontal scroll bar, with resize handles that allow the user to resize the visible
    /// scroll area.
    /// </summary>
    public sealed class GUIResizeableScrollBarH : GUIScrollBar
    {
        /// <summary>
        /// Event triggered whenever the user moves or resizes the scroll bar. Value reported is the position and size of
        /// the scroll bar handle in percent, in [0, 1] range.
        /// </summary>
        public event Action<float, float> OnScrollOrResize;

        /// <summary>
        /// Creates a new resizeable horizontal scroll bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIResizeableScrollBarH(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, style, options);
        }

        /// <summary>
        /// Creates a new resizeable horizontal scroll bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUIResizeableScrollBarH(string style = "")
        {
            Internal_CreateInstance(this, style, new GUIOption[0]);
        }

        private void Internal_DoOnScrollOrResize(float percent, float size)
        {
            if (OnScrollOrResize != null)
                OnScrollOrResize(percent, size);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIResizeableScrollBarH instance, string style, GUIOption[] options);
    }

    /// <summary>
    /// GUI element representing a vertical scroll bar, with resize handles that allow the user to resize the visible
    /// scroll area.
    /// </summary>
    public sealed class GUIResizeableScrollBarV : GUIScrollBar
    {
        /// <summary>
        /// Event triggered whenever the user moves or resizes the scroll bar. Value reported is the position and size of
        /// the scroll bar handle in percent, in [0, 1] range.
        /// </summary>
        public event Action<float, float> OnScrollOrResize;

        /// <summary>
        /// Creates a new resizeable vertical scroll bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIResizeableScrollBarV(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, style, options);
        }

        /// <summary>
        /// Creates a new resizeable vertical scroll bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUIResizeableScrollBarV(string style = "")
        {
            Internal_CreateInstance(this, style, new GUIOption[0]);
        }

        private void Internal_DoOnScrollOrResize(float percent, float size)
        {
            if (OnScrollOrResize != null)
                OnScrollOrResize(percent, size);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIResizeableScrollBarV instance, string style, GUIOption[] options);
    }

    /** @} */
}

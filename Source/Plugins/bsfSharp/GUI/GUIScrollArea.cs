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
    /// Scroll bar options for a GUI scroll area.
    /// </summary>
    public enum ScrollBarType // Note: Must match the C++ enum ScrollBarType
    {
        ShowIfDoesntFit,
        AlwaysShow,
        NeverShow
    };

    /// <summary>
    /// A GUI element container with support for vertical & horizontal scrolling.
    /// </summary>
    public sealed class GUIScrollArea : GUIElement
    {
        private GUILayout _mainLayout;

        /// <summary>
        /// Default vertical layout into which you should place elements that need to be scrollable.
        /// </summary>
        public GUILayout Layout
        {
            get { return _mainLayout; }
        }

        /// <summary>
        /// Position of the horizontal scroll bar, ranging [0, 1]. When bar is fully to the left (no scroll) the value is 
        /// zero, and when the bar is fully to the right (full scroll) the value is one.
        /// </summary>
        public float HorizontalScroll
        {
            get { return Internal_GetHorzScroll(mCachedPtr); }
            set { Internal_SetHorzScroll(mCachedPtr, value); }
        }

        /// <summary>
        /// Position of the vertical scroll bar, ranging [0, 1]. When bar is fully to the top (no scroll) the value is 
        /// zero, and when the bar is fully to the bottom (full scroll) the value is one.
        /// </summary>
        public float VerticalScroll
        {
            get { return Internal_GetVertScroll(mCachedPtr); }
            set { Internal_SetVertScroll(mCachedPtr, value); }
        }

        /// <summary>
        /// Returns the bounds of the scroll area not including the scroll bars (only the portion that contains the 
        /// contents).
        /// </summary>
        public Rect2I ContentBounds
        {
            get
            {
                Rect2I bounds;
                Internal_GetContentBounds(mCachedPtr, out bounds);
                return bounds;
            }
        }

        /// <summary>
        /// Number of pixels the scroll bar will occupy when active. This is width for vertical scrollbar, and height for 
        /// horizontal scrollbar.
        /// </summary>
        public int ScrollBarWidth
        {
            get { return Internal_GetScrollBarWidth(mCachedPtr); }
        }

        /// <summary>
        /// Creates a new scroll area.
        /// </summary>
        /// <param name="vertBarType">Type of the vertical scroll bar.</param>
        /// <param name="horzBarType">Type of the horizontal scroll bar.</param>
        /// <param name="scrollBarStyle">Optional style that controls the look of the scroll bars. Style will be retrieved 
        ///                              from the active GUISkin. If not specified default element style is used.</param>        
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIScrollArea(ScrollBarType vertBarType, ScrollBarType horzBarType, string scrollBarStyle,
            string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, vertBarType, horzBarType, scrollBarStyle, style, options);
            _mainLayout = new GUILayoutY(this);
        }

        /// <summary>
        /// Creates a new scroll area.
        /// </summary>
        /// <param name="vertBarType">Type of the vertical scroll bar.</param>
        /// <param name="horzBarType">Type of the horizontal scroll bar.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIScrollArea(ScrollBarType vertBarType, ScrollBarType horzBarType, string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, vertBarType, horzBarType, "", style, options);
            _mainLayout = new GUILayoutY(this);
        }

        /// <summary>
        /// Creates a new scroll area.
        /// </summary>
        /// <param name="vertBarType">Type of the vertical scroll bar.</param>
        /// <param name="horzBarType">Type of the horizontal scroll bar.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIScrollArea(ScrollBarType vertBarType, ScrollBarType horzBarType, params GUIOption[] options)
        {
            Internal_CreateInstance(this, vertBarType, horzBarType, "", "", options);
            _mainLayout = new GUILayoutY(this);
        }

        /// <summary>
        /// Creates a new scroll area.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIScrollArea(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ScrollBarType.ShowIfDoesntFit, ScrollBarType.ShowIfDoesntFit, "", style, options);
            _mainLayout = new GUILayoutY(this);
        }

        /// <summary>
        /// Creates a new scroll area.
        /// </summary>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIScrollArea(params GUIOption[] options)
        {
            Internal_CreateInstance(this, ScrollBarType.ShowIfDoesntFit, ScrollBarType.ShowIfDoesntFit, "", "", options);
            _mainLayout = new GUILayoutY(this);
        }

        /// <summary>
        /// Creates a new scroll area.
        /// </summary>
        /// <param name="scrollBarStyle">Optional style that controls the look of the scroll bars. Style will be retrieved 
        ///                              from the active GUISkin. If not specified default element style is used.</param> 
        /// <param name="scrollAreaStyle">Optional style that controls the look of the scroll area. Style will be retrieved 
        ///                              from the active GUISkin. If not specified default element style is used.</param>      
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIScrollArea(string scrollBarStyle, string scrollAreaStyle, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ScrollBarType.ShowIfDoesntFit, ScrollBarType.ShowIfDoesntFit, scrollBarStyle, scrollAreaStyle, options);
            _mainLayout = new GUILayoutY(this);
        }

        /// <inheritdoc/>
        public override void Destroy()
        {
            _mainLayout.Destroy();

            base.Destroy();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIScrollArea instance, ScrollBarType vertBarType, ScrollBarType horzBarType,
            string scrollBarStyle, string scrollAreaStyle, params GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetContentBounds(IntPtr nativeInstance, out Rect2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetHorzScroll(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetHorzScroll(IntPtr nativeInstance, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetVertScroll(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVertScroll(IntPtr nativeInstance, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetScrollBarWidth(IntPtr nativeInstance);
    }

    /** @} */
}

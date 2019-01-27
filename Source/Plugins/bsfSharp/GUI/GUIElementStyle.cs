//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup GUI_Engine
     *  @{
     */

    /// <summary>
    /// Rectangle represented in the form of offsets from some parent rectangle.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct RectOffset // Note: Must match the C++ struct RectOffset
    {
        public int left, right, top, bottom;
    };

    /// <summary>
    /// Possible positions used for positioning content image within a GUI element.
    /// </summary>
    public enum GUIImagePosition // Note: Must match the C++ enum GUIImagePosition.
    {
        Left, Right
    };

    /// <summary>
    /// Specifies how is text horizontally aligned within its bounds.
    /// </summary>
    public enum TextHorzAlign // Note: Must match the C++ enum TextHorzAlign.
    {
        Left, Center, Right
    };

    /// <summary>
    /// Specifies how is text vertically aligned within its bounds.
    /// </summary>
    public enum TextVertAlign // Note: Must match the C++ enum TextVertAlign
    {
        Top, Center, Bottom
    };

    /// <summary>
    /// GUI element style that determines the look of a GUI element, as well as the element's default layout options. 
    /// Different looks can be provided for different element states.
    /// </summary>
    [SerializeObject]
    public sealed class GUIElementStyle : ScriptObject
    {
        // Constructor for runtime use only (dummy parameter to differentiate from the normal constructor)
        private GUIElementStyle(bool dummy)
        {  }

        /// <summary>
        /// Constructs a new GUI element style with default values.
        /// </summary>
        public GUIElementStyle()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Font to use for all text within the GUI element. 
        /// </summary>
        public Font Font
        {
            get { Font value; Internal_GetFont(mCachedPtr, out value); return value; }
            set { Internal_SetFont(mCachedPtr, value); }
        }

        /// <summary>
        /// Font size to use for all text within the GUI element.
        /// </summary>
        public int FontSize
        {
            get { int value; Internal_GetFontSize(mCachedPtr, out value); return value; }
            set { Internal_SetFontSize(mCachedPtr, value); }
        }

        /// <summary>
        /// Horizontal alignment of text within the GUI element. 
        /// </summary>
        public TextHorzAlign TextHorzAlign
        {
            get { TextHorzAlign value; Internal_GetTextHorzAlign(mCachedPtr, out value); return value; }
            set { Internal_SetTextHorzAlign(mCachedPtr, value); }
        }

        /// <summary>
        /// Vertical alignment of text within the GUI element. 
        /// </summary>
        public TextVertAlign TextVertAlign
        {
            get { TextVertAlign value; Internal_GetTextVertAlign(mCachedPtr, out value); return value; }
            set { Internal_SetTextVertAlign(mCachedPtr, value); }
        }

        /// <summary>
        /// Position of content image relative to text.
        /// </summary>
        public GUIImagePosition ImagePosition
        {
            get { GUIImagePosition value; Internal_GetImagePosition(mCachedPtr, out value); return value; }
            set { Internal_SetImagePosition(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the text word wrap if it doesn't fit in its element's bounds.
        /// </summary>
        public bool WordWrap
        {
            get { bool value; Internal_GetWordWrap(mCachedPtr, out value); return value; }
            set { Internal_SetWordWrap(mCachedPtr, value); }
        }


        /// <summary>
        /// Style used when element is in normal state and off.
        /// </summary>
        public GUIElementStateStyle Normal
        {
            get { GUIElementStateStyle value; Internal_GetNormal(mCachedPtr, out value); return value; }
            set { Internal_SetNormal(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Style used when element is in hover state and off.
        /// </summary>
        public GUIElementStateStyle Hover
        {
            get { GUIElementStateStyle value; Internal_GetHover(mCachedPtr, out value); return value; }
            set { Internal_SetHover(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Style used when element is in active state and off.
        /// </summary>
        public GUIElementStateStyle Active
        {
            get { GUIElementStateStyle value; Internal_GetActive(mCachedPtr, out value); return value; }
            set { Internal_SetActive(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Style used when element is in focused state and off.
        /// </summary>
        public GUIElementStateStyle Focused
        {
            get { GUIElementStateStyle value; Internal_GetFocused(mCachedPtr, out value); return value; }
            set { Internal_SetFocused(mCachedPtr, ref value); }
        }


        /// <summary>
        /// Style used when element is in normal state and on.
        /// </summary>
        public GUIElementStateStyle NormalOn
        {
            get { GUIElementStateStyle value; Internal_GetNormalOn(mCachedPtr, out value); return value; }
            set { Internal_SetNormalOn(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Style used when element is in hover state and on.
        /// </summary>
        public GUIElementStateStyle HoverOn
        {
            get { GUIElementStateStyle value; Internal_GetHoverOn(mCachedPtr, out value); return value; }
            set { Internal_SetHoverOn(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Style used when element is in active state and on.
        /// </summary>
        public GUIElementStateStyle ActiveOn
        {
            get { GUIElementStateStyle value; Internal_GetActiveOn(mCachedPtr, out value); return value; }
            set { Internal_SetActiveOn(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Style used when element is in focused state and on. 
        /// </summary>
        public GUIElementStateStyle FocusedOn
        {
            get { GUIElementStateStyle value; Internal_GetFocusedOn(mCachedPtr, out value); return value; }
            set { Internal_SetFocusedOn(mCachedPtr, ref value); }
        }


        /// <summary>
        /// Determines how the element is scaled (using the typical Scale9Grid approach).
        /// </summary>
        public RectOffset Border
        {
            get { RectOffset value; Internal_GetBorder(mCachedPtr, out value); return value; }
            set { Internal_SetBorder(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Determines offset from the background graphics to the content. Input uses bounds offset by this value.
        /// </summary>
        public RectOffset Margins
        {
            get { RectOffset value; Internal_GetMargins(mCachedPtr, out value); return value; }
            set { Internal_SetMargins(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Additional offset to the content, that doesn't effect the bounds. Applied on top of the margins offsets.
        /// </summary>
        public RectOffset ContentOffset
        {
            get { RectOffset value; Internal_GetContentOffset(mCachedPtr, out value); return value; }
            set { Internal_SetContentOffset(mCachedPtr, ref value); }
        }


        /// <summary>
        /// Wanted width of the GUI element in pixels. Only used if <see cref="FixedWidth"/> is enabled.
        /// </summary>
        public int Width
        {
            get { int value; Internal_GetWidth(mCachedPtr, out value); return value; }
            set { Internal_SetWidth(mCachedPtr, value); }
        }

        /// <summary>
        /// Wanted height of the GUI element in pixels. Only used if <see cref="FixedHeight"/> is enabled.
        /// </summary>
        public int Height
        {
            get { int value; Internal_GetHeight(mCachedPtr, out value); return value; }
            set { Internal_SetHeight(mCachedPtr, value); }
        }

        /// <summary>
        /// Minimum width allowed for the GUI element. Used by the layout only when exact width is not specified.
        /// </summary>
        public int MinWidth
        {
            get { int value; Internal_GetMinWidth(mCachedPtr, out value); return value; }
            set { Internal_SetMinWidth(mCachedPtr, value); }
        }

        /// <summary>
        /// Maximum width allowed for the GUI element. Used by the layout only when exact width is not specified.
        /// </summary>
        public int MaxWidth
        {
            get { int value; Internal_GetMaxWidth(mCachedPtr, out value); return value; }
            set { Internal_SetMaxWidth(mCachedPtr, value); }
        }

        /// <summary>
        /// Minimum height allowed for the GUI element. Used by the layout only when exact height is not specified.
        /// </summary>
        public int MinHeight
        {
            get { int value; Internal_GetMinHeight(mCachedPtr, out value); return value; }
            set { Internal_SetMinHeight(mCachedPtr, value); }
        }

        /// <summary>
        /// Maximum height allowed for the GUI element. Used by the layout only when exact height is not specified.
        /// </summary>
        public int MaxHeight
        {
            get { int value; Internal_GetMaxHeight(mCachedPtr, out value); return value; }
            set { Internal_SetMaxHeight(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the layout resize the element depending on available size. If true no resizing will be done.
        /// </summary>
        public bool FixedWidth
        {
            get { bool value; Internal_GetFixedWidth(mCachedPtr, out value); return value; }
            set { Internal_SetFixedWidth(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the layout resize the element depending on available size. If true no resizing will be done.
        /// </summary>
        public bool FixedHeight
        {
            get { bool value; Internal_GetFixedHeight(mCachedPtr, out value); return value; }
            set { Internal_SetFixedHeight(mCachedPtr, value); }
        }

        /// <summary>
        /// Registers a new sub-style that is used by complex GUI elements that contain one or multiple sub-elements.
        /// </summary>
        /// <param name="guiType">Name of the sub-element this style is to be used for.  This depends on GUI element the 
        ///                       style is applied to.</param>
        /// <param name="styleName">Name of the style in GUI skin to use for the sub-element.</param>
        public void AddSubStyle(string guiType, string styleName)
        {
            if (guiType == null || styleName == null)
                return;

            Internal_AddSubStyle(mCachedPtr, guiType, styleName);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIElementStyle instance);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetFont(IntPtr nativeInstance, out Font value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetFont(IntPtr nativeInstance, Font value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetFontSize(IntPtr nativeInstance, out int value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetFontSize(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetTextHorzAlign(IntPtr nativeInstance, out TextHorzAlign value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetTextHorzAlign(IntPtr nativeInstance, TextHorzAlign value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetTextVertAlign(IntPtr nativeInstance, out TextVertAlign value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetTextVertAlign(IntPtr nativeInstance, TextVertAlign value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetImagePosition(IntPtr nativeInstance, out GUIImagePosition value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetImagePosition(IntPtr nativeInstance, GUIImagePosition value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetWordWrap(IntPtr nativeInstance, out bool value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetWordWrap(IntPtr nativeInstance, bool value);


        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetNormal(IntPtr nativeInstance, out GUIElementStateStyle value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetNormal(IntPtr nativeInstance, ref GUIElementStateStyle value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetHover(IntPtr nativeInstance, out GUIElementStateStyle value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetHover(IntPtr nativeInstance, ref GUIElementStateStyle value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetActive(IntPtr nativeInstance, out GUIElementStateStyle value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetActive(IntPtr nativeInstance, ref GUIElementStateStyle value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetFocused(IntPtr nativeInstance, out GUIElementStateStyle value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetFocused(IntPtr nativeInstance, ref GUIElementStateStyle value);


        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetNormalOn(IntPtr nativeInstance, out GUIElementStateStyle value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetNormalOn(IntPtr nativeInstance, ref GUIElementStateStyle value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetHoverOn(IntPtr nativeInstance, out GUIElementStateStyle value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetHoverOn(IntPtr nativeInstance, ref GUIElementStateStyle value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetActiveOn(IntPtr nativeInstance, out GUIElementStateStyle value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetActiveOn(IntPtr nativeInstance, ref GUIElementStateStyle value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetFocusedOn(IntPtr nativeInstance, out GUIElementStateStyle value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetFocusedOn(IntPtr nativeInstance, ref GUIElementStateStyle value);


        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetBorder(IntPtr nativeInstance, out RectOffset value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetBorder(IntPtr nativeInstance, ref RectOffset value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetMargins(IntPtr nativeInstance, out RectOffset value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetMargins(IntPtr nativeInstance, ref RectOffset value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetContentOffset(IntPtr nativeInstance, out RectOffset value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetContentOffset(IntPtr nativeInstance, ref RectOffset value);


        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetWidth(IntPtr nativeInstance, out int value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetWidth(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetHeight(IntPtr nativeInstance, out int value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetHeight(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetMinWidth(IntPtr nativeInstance, out int value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetMinWidth(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetMaxWidth(IntPtr nativeInstance, out int value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetMaxWidth(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetMinHeight(IntPtr nativeInstance, out int value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetMinHeight(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetMaxHeight(IntPtr nativeInstance, out int value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetMaxHeight(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetFixedWidth(IntPtr nativeInstance, out bool value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetFixedWidth(IntPtr nativeInstance, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_GetFixedHeight(IntPtr nativeInstance, out bool value);
        [MethodImpl(MethodImplOptions.InternalCall)] 
        private static extern void Internal_SetFixedHeight(IntPtr nativeInstance, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_AddSubStyle(IntPtr nativeInstance, string guiType, string styleName);
    }

    /** @} */
}

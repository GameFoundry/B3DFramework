//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>
	/// GUI element style that determines the look of a GUI element, as well as the element&apos;s default layout options. 
	/// Different looks can be provided for different element states.
	/// </summary>
	[ShowInInspector]
	public partial class GUIElementStyle : ScriptObject
	{
		private GUIElementStyle(bool __dummy0) { }

		public GUIElementStyle()
		{
			Internal_GUIElementStyle(this);
		}

		/// <summary>Font to use for all text within the GUI element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Font Font
		{
			get { return Internal_GetFont(mCachedPtr); }
			set { Internal_SetFont(mCachedPtr, value); }
		}

		/// <summary>Font size to use for all text within the GUI element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int FontSize
		{
			get { return Internal_GetFontSize(mCachedPtr); }
			set { Internal_SetFontSize(mCachedPtr, value); }
		}

		/// <summary>Horizontal alignment of text within the GUI element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public TextHorzAlign TextHorzAlign
		{
			get { return Internal_GetTextHorzAlign(mCachedPtr); }
			set { Internal_SetTextHorzAlign(mCachedPtr, value); }
		}

		/// <summary>Vertical alignment of text within the GUI element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public TextVertAlign TextVertAlign
		{
			get { return Internal_GetTextVertAlign(mCachedPtr); }
			set { Internal_SetTextVertAlign(mCachedPtr, value); }
		}

		/// <summary>Position of content image relative to text.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIImagePosition ImagePosition
		{
			get { return Internal_GetImagePosition(mCachedPtr); }
			set { Internal_SetImagePosition(mCachedPtr, value); }
		}

		/// <summary>Should the text word wrap if it doesn&apos;t fit.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool WordWrap
		{
			get { return Internal_GetWordWrap(mCachedPtr); }
			set { Internal_SetWordWrap(mCachedPtr, value); }
		}

		/// <summary>
		/// Style used when the element doesn&apos;t have focus nor is the user interacting with the element. Used when the 
		/// element is in the &apos;off&apos; state.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle Normal
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetNormal(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetNormal(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Style used when the user is hovering the pointer over the element, while the element doesn&apos;t have focus. Used 
		/// when the element is in the &apos;off&apos; state.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle Hover
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetHover(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetHover(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Style used when the user is actively interacting with the element. Used when the element is in the &apos;off&apos; 
		/// state.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle Active
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetActive(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetActive(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Style used when the element has focus but the pointer is not hovering over the element. Used when the element is in 
		/// the &apos;off&apos; state.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle Focused
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetFocused(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetFocused(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Style used when the element has focus and the pointer is hovering over the element. Used when the element is in the 
		/// &apos;off&apos; state.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle FocusedHover
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetFocusedHover(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetFocusedHover(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::normal, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle NormalOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetNormalOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetNormalOn(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::hover, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle HoverOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetHoverOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetHoverOn(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::active, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle ActiveOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetActiveOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetActiveOn(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::focused, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle FocusedOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetFocusedOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetFocusedOn(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::focusedHover, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle FocusedHoverOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_GetFocusedHoverOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetFocusedHoverOn(mCachedPtr, ref value); }
		}

		/// <summary>Determines how the element is scaled (using the typical Scale9Grid approach).</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RectOffset Border
		{
			get
			{
				RectOffset temp;
				Internal_GetBorder(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetBorder(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines offset from the background graphics to the content. Input uses bounds offset by this value.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RectOffset Margins
		{
			get
			{
				RectOffset temp;
				Internal_GetMargins(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetMargins(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Additional offset to the content, that doesn&apos;t effect the bounds. Applied on top of the margins offsets.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RectOffset ContentOffset
		{
			get
			{
				RectOffset temp;
				Internal_GetContentOffset(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetContentOffset(mCachedPtr, ref value); }
		}

		/// <summary>Determines extra distance between this and other elements in a layout.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RectOffset Padding
		{
			get
			{
				RectOffset temp;
				Internal_GetPadding(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetPadding(mCachedPtr, ref value); }
		}

		/// <summary>Wanted width of the GUI element in pixels. Only used if fixedWidth is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Width
		{
			get { return Internal_GetWidth(mCachedPtr); }
			set { Internal_SetWidth(mCachedPtr, value); }
		}

		/// <summary>Wanted height of the GUI element in pixels. Only used if fixedHeight is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Height
		{
			get { return Internal_GetHeight(mCachedPtr); }
			set { Internal_SetHeight(mCachedPtr, value); }
		}

		/// <summary>
		/// Minimum width allowed for the GUI element. Used by the layout only when exact width is not specified.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MinWidth
		{
			get { return Internal_GetMinWidth(mCachedPtr); }
			set { Internal_SetMinWidth(mCachedPtr, value); }
		}

		/// <summary>
		/// Maximum width allowed for the GUI element. Used by the layout only when exact width is not specified.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MaxWidth
		{
			get { return Internal_GetMaxWidth(mCachedPtr); }
			set { Internal_SetMaxWidth(mCachedPtr, value); }
		}

		/// <summary>
		/// Minimum height allowed for the GUI element. Used by the layout only when exact height is not specified.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MinHeight
		{
			get { return Internal_GetMinHeight(mCachedPtr); }
			set { Internal_SetMinHeight(mCachedPtr, value); }
		}

		/// <summary>
		/// Maximum height allowed for the GUI element. Used by the layout only when exact height is not specified.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MaxHeight
		{
			get { return Internal_GetMaxHeight(mCachedPtr); }
			set { Internal_SetMaxHeight(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the layout resize the element depending on available size. If true no resizing will be done.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool FixedWidth
		{
			get { return Internal_GetFixedWidth(mCachedPtr); }
			set { Internal_SetFixedWidth(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the layout resize the element depending on available size. If true no resizing will be done.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool FixedHeight
		{
			get { return Internal_GetFixedHeight(mCachedPtr); }
			set { Internal_SetFixedHeight(mCachedPtr, value); }
		}

		/// <summary>
		/// Registers a new sub-style that is used by complex GUI elements that contain one or multiple sub-elements.
		/// </summary>
		/// <param name="guiType">
		/// Name of the sub-element this style is to be used for. This depends on GUI element the style is applied to.
		/// </param>
		/// <param name="styleName">Name of the style in GUI skin to use for the sub-element.</param>
		public void AddSubStyle(string guiType, string styleName)
		{
			Internal_AddSubStyle(mCachedPtr, guiType, styleName);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GUIElementStyle(GUIElementStyle managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AddSubStyle(IntPtr thisPtr, string guiType, string styleName);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Font Internal_GetFont(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFont(IntPtr thisPtr, Font value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetFontSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFontSize(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TextHorzAlign Internal_GetTextHorzAlign(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTextHorzAlign(IntPtr thisPtr, TextHorzAlign value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TextVertAlign Internal_GetTextVertAlign(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTextVertAlign(IntPtr thisPtr, TextVertAlign value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern GUIImagePosition Internal_GetImagePosition(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImagePosition(IntPtr thisPtr, GUIImagePosition value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetWordWrap(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetWordWrap(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetNormal(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNormal(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetHover(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHover(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetActive(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetActive(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetFocused(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFocused(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetFocusedHover(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFocusedHover(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetNormalOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNormalOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetHoverOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHoverOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetActiveOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetActiveOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetFocusedOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFocusedOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetFocusedHoverOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFocusedHoverOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetBorder(IntPtr thisPtr, out RectOffset __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBorder(IntPtr thisPtr, ref RectOffset value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetMargins(IntPtr thisPtr, out RectOffset __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMargins(IntPtr thisPtr, ref RectOffset value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetContentOffset(IntPtr thisPtr, out RectOffset __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetContentOffset(IntPtr thisPtr, ref RectOffset value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetPadding(IntPtr thisPtr, out RectOffset __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPadding(IntPtr thisPtr, ref RectOffset value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetWidth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHeight(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetMinWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMinWidth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetMaxWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxWidth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetMinHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMinHeight(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetMaxHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxHeight(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetFixedWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFixedWidth(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetFixedHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFixedHeight(IntPtr thisPtr, bool value);
	}

	/** @} */
}

//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
			get { return Internal_getfont(mCachedPtr); }
			set { Internal_setfont(mCachedPtr, value); }
		}

		/// <summary>Font size to use for all text within the GUI element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int FontSize
		{
			get { return Internal_getfontSize(mCachedPtr); }
			set { Internal_setfontSize(mCachedPtr, value); }
		}

		/// <summary>Horizontal alignment of text within the GUI element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public TextHorzAlign TextHorzAlign
		{
			get { return Internal_gettextHorzAlign(mCachedPtr); }
			set { Internal_settextHorzAlign(mCachedPtr, value); }
		}

		/// <summary>Vertical alignment of text within the GUI element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public TextVertAlign TextVertAlign
		{
			get { return Internal_gettextVertAlign(mCachedPtr); }
			set { Internal_settextVertAlign(mCachedPtr, value); }
		}

		/// <summary>Position of content image relative to text.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIImagePosition ImagePosition
		{
			get { return Internal_getimagePosition(mCachedPtr); }
			set { Internal_setimagePosition(mCachedPtr, value); }
		}

		/// <summary>Should the text word wrap if it doesn&apos;t fit.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool WordWrap
		{
			get { return Internal_getwordWrap(mCachedPtr); }
			set { Internal_setwordWrap(mCachedPtr, value); }
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
				Internal_getnormal(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setnormal(mCachedPtr, ref value); }
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
				Internal_gethover(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_sethover(mCachedPtr, ref value); }
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
				Internal_getactive(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setactive(mCachedPtr, ref value); }
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
				Internal_getfocused(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setfocused(mCachedPtr, ref value); }
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
				Internal_getfocusedHover(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setfocusedHover(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::normal, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle NormalOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_getnormalOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setnormalOn(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::hover, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle HoverOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_gethoverOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_sethoverOn(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::active, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle ActiveOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_getactiveOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setactiveOn(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::focused, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle FocusedOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_getfocusedOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setfocusedOn(mCachedPtr, ref value); }
		}

		/// <summary>Same as GUIElementStyle::focusedHover, except it&apos;s used when element is in the &apos;on&apos; state.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementStateStyle FocusedHoverOn
		{
			get
			{
				GUIElementStateStyle temp;
				Internal_getfocusedHoverOn(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setfocusedHoverOn(mCachedPtr, ref value); }
		}

		/// <summary>Determines how the element is scaled (using the typical Scale9Grid approach).</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RectOffset Border
		{
			get
			{
				RectOffset temp;
				Internal_getborder(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setborder(mCachedPtr, ref value); }
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
				Internal_getmargins(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setmargins(mCachedPtr, ref value); }
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
				Internal_getcontentOffset(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setcontentOffset(mCachedPtr, ref value); }
		}

		/// <summary>Determines extra distance between this and other elements in a layout.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RectOffset Padding
		{
			get
			{
				RectOffset temp;
				Internal_getpadding(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setpadding(mCachedPtr, ref value); }
		}

		/// <summary>Wanted width of the GUI element in pixels. Only used if fixedWidth is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Width
		{
			get { return Internal_getwidth(mCachedPtr); }
			set { Internal_setwidth(mCachedPtr, value); }
		}

		/// <summary>Wanted height of the GUI element in pixels. Only used if fixedHeight is enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Height
		{
			get { return Internal_getheight(mCachedPtr); }
			set { Internal_setheight(mCachedPtr, value); }
		}

		/// <summary>
		/// Minimum width allowed for the GUI element. Used by the layout only when exact width is not specified.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MinWidth
		{
			get { return Internal_getminWidth(mCachedPtr); }
			set { Internal_setminWidth(mCachedPtr, value); }
		}

		/// <summary>
		/// Maximum width allowed for the GUI element. Used by the layout only when exact width is not specified.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MaxWidth
		{
			get { return Internal_getmaxWidth(mCachedPtr); }
			set { Internal_setmaxWidth(mCachedPtr, value); }
		}

		/// <summary>
		/// Minimum height allowed for the GUI element. Used by the layout only when exact height is not specified.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MinHeight
		{
			get { return Internal_getminHeight(mCachedPtr); }
			set { Internal_setminHeight(mCachedPtr, value); }
		}

		/// <summary>
		/// Maximum height allowed for the GUI element. Used by the layout only when exact height is not specified.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MaxHeight
		{
			get { return Internal_getmaxHeight(mCachedPtr); }
			set { Internal_setmaxHeight(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the layout resize the element depending on available size. If true no resizing will be done.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool FixedWidth
		{
			get { return Internal_getfixedWidth(mCachedPtr); }
			set { Internal_setfixedWidth(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the layout resize the element depending on available size. If true no resizing will be done.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool FixedHeight
		{
			get { return Internal_getfixedHeight(mCachedPtr); }
			set { Internal_setfixedHeight(mCachedPtr, value); }
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
			Internal_addSubStyle(mCachedPtr, guiType, styleName);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GUIElementStyle(GUIElementStyle managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_addSubStyle(IntPtr thisPtr, string guiType, string styleName);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Font Internal_getfont(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfont(IntPtr thisPtr, Font value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getfontSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfontSize(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TextHorzAlign Internal_gettextHorzAlign(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settextHorzAlign(IntPtr thisPtr, TextHorzAlign value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TextVertAlign Internal_gettextVertAlign(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settextVertAlign(IntPtr thisPtr, TextVertAlign value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern GUIImagePosition Internal_getimagePosition(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setimagePosition(IntPtr thisPtr, GUIImagePosition value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getwordWrap(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setwordWrap(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getnormal(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setnormal(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_gethover(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethover(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getactive(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setactive(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getfocused(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfocused(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getfocusedHover(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfocusedHover(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getnormalOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setnormalOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_gethoverOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethoverOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getactiveOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setactiveOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getfocusedOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfocusedOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getfocusedHoverOn(IntPtr thisPtr, out GUIElementStateStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfocusedHoverOn(IntPtr thisPtr, ref GUIElementStateStyle value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getborder(IntPtr thisPtr, out RectOffset __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setborder(IntPtr thisPtr, ref RectOffset value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getmargins(IntPtr thisPtr, out RectOffset __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmargins(IntPtr thisPtr, ref RectOffset value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getcontentOffset(IntPtr thisPtr, out RectOffset __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcontentOffset(IntPtr thisPtr, ref RectOffset value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getpadding(IntPtr thisPtr, out RectOffset __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setpadding(IntPtr thisPtr, ref RectOffset value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getwidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setwidth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getheight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setheight(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getminWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setminWidth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getmaxWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaxWidth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getminHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setminHeight(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getmaxHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaxHeight(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getfixedWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfixedWidth(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getfixedHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfixedHeight(IntPtr thisPtr, bool value);
	}

	/** @} */
}

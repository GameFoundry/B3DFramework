//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Text
	 *  @{
	 */

	/// <summary>Import options that allow you to control how is a font imported.</summary>
	[ShowInInspector]
	public partial class FontImportOptions : ImportOptions
	{
		private FontImportOptions(bool __dummy0) { }

		/// <summary>Creates a new import options object that allows you to customize how are fonts imported.</summary>
		public FontImportOptions()
		{
			Internal_Create(this);
		}

		/// <summary>Determines font sizes that are to be imported. Sizes are in points.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float[] FontSizes
		{
			get { return Internal_GetFontSizes(mCachedPtr); }
			set { Internal_SetFontSizes(mCachedPtr, value); }
		}

		/// <summary>Determines character index ranges to import. Ranges are defined as unicode numbers.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CharRange[] CharIndexRanges
		{
			get { return Internal_GetCharIndexRanges(mCachedPtr); }
			set { Internal_SetCharIndexRanges(mCachedPtr, value); }
		}

		/// <summary>Determines dots per inch scale that will be used when rendering the characters.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Dpi
		{
			get { return Internal_GetDpi(mCachedPtr); }
			set { Internal_SetDpi(mCachedPtr, value); }
		}

		/// <summary>Determines the render mode used for rendering the characters into a bitmap.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public FontRenderMode RenderMode
		{
			get { return Internal_GetRenderMode(mCachedPtr); }
			set { Internal_SetRenderMode(mCachedPtr, value); }
		}

		/// <summary>Determines whether the bold font style should be used when rendering.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Bold
		{
			get { return Internal_GetBold(mCachedPtr); }
			set { Internal_SetBold(mCachedPtr, value); }
		}

		/// <summary>Determines whether the italic font style should be used when rendering.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Italic
		{
			get { return Internal_GetItalic(mCachedPtr); }
			set { Internal_SetItalic(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float[] Internal_GetFontSizes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFontSizes(IntPtr thisPtr, float[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CharRange[] Internal_GetCharIndexRanges(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCharIndexRanges(IntPtr thisPtr, CharRange[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetDpi(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDpi(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FontRenderMode Internal_GetRenderMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRenderMode(IntPtr thisPtr, FontRenderMode value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetBold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBold(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetItalic(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetItalic(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(FontImportOptions managedInstance);
	}

	/** @} */
#endif
}

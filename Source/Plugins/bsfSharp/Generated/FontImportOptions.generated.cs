//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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
			Internal_create(this);
		}

		/// <summary>Determines font sizes that are to be imported. Sizes are in points.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int[] FontSizes
		{
			get { return Internal_getfontSizes(mCachedPtr); }
			set { Internal_setfontSizes(mCachedPtr, value); }
		}

		/// <summary>Determines character index ranges to import. Ranges are defined as unicode numbers.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CharRange[] CharIndexRanges
		{
			get { return Internal_getcharIndexRanges(mCachedPtr); }
			set { Internal_setcharIndexRanges(mCachedPtr, value); }
		}

		/// <summary>Determines dots per inch scale that will be used when rendering the characters.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Dpi
		{
			get { return Internal_getdpi(mCachedPtr); }
			set { Internal_setdpi(mCachedPtr, value); }
		}

		/// <summary>Determines the render mode used for rendering the characters into a bitmap.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public FontRenderMode RenderMode
		{
			get { return Internal_getrenderMode(mCachedPtr); }
			set { Internal_setrenderMode(mCachedPtr, value); }
		}

		/// <summary>Determines whether the bold font style should be used when rendering.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Bold
		{
			get { return Internal_getbold(mCachedPtr); }
			set { Internal_setbold(mCachedPtr, value); }
		}

		/// <summary>Determines whether the italic font style should be used when rendering.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Italic
		{
			get { return Internal_getitalic(mCachedPtr); }
			set { Internal_setitalic(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int[] Internal_getfontSizes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfontSizes(IntPtr thisPtr, int[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CharRange[] Internal_getcharIndexRanges(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcharIndexRanges(IntPtr thisPtr, CharRange[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getdpi(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setdpi(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FontRenderMode Internal_getrenderMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setrenderMode(IntPtr thisPtr, FontRenderMode value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getbold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setbold(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getitalic(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setitalic(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(FontImportOptions managedInstance);
	}

	/** @} */
#endif
}

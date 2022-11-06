//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI_Engine
	 *  @{
	 */

	/// <summary>Contains textures and data about every character for a bitmap font of a specific size.</summary>
	[ShowInInspector]
	public partial class FontBitmap : ScriptObject
	{
		private FontBitmap(bool __dummy0) { }
		protected FontBitmap() { }

		/// <summary>Font size for which the data is contained.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Size
		{
			get { return Internal_GetSize(mCachedPtr); }
			set { Internal_SetSize(mCachedPtr, value); }
		}

		/// <summary>Y offset to the baseline on which the characters are placed, in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int BaselineOffset
		{
			get { return Internal_GetBaselineOffset(mCachedPtr); }
			set { Internal_SetBaselineOffset(mCachedPtr, value); }
		}

		/// <summary>Height of a single line of the font, in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int LineHeight
		{
			get { return Internal_GetLineHeight(mCachedPtr); }
			set { Internal_SetLineHeight(mCachedPtr, value); }
		}

		/// <summary>Character to use when data for a character is missing.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CharDesc MissingGlyph
		{
			get
			{
				CharDesc temp;
				Internal_GetMissingGlyph(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetMissingGlyph(mCachedPtr, ref value); }
		}

		/// <summary>Width of a space in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public int SpaceWidth
		{
			get { return Internal_GetSpaceWidth(mCachedPtr); }
			set { Internal_SetSpaceWidth(mCachedPtr, value); }
		}

		/// <summary>Textures in which the character&apos;s pixels are stored.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Texture>[] TexturePages
		{
			get { return Internal_GetTexturePages(mCachedPtr); }
			set { Internal_SetTexturePages(mCachedPtr, value); }
		}

		/// <summary>Returns a character description for the character with the specified Unicode key.</summary>
		public CharDesc GetCharDesc(int charId)
		{
			CharDesc temp;
			Internal_GetCharDesc(mCachedPtr, charId, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCharDesc(IntPtr thisPtr, int charId, out CharDesc __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSize(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetBaselineOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBaselineOffset(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetLineHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLineHeight(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetMissingGlyph(IntPtr thisPtr, out CharDesc __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMissingGlyph(IntPtr thisPtr, ref CharDesc value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetSpaceWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpaceWidth(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture>[] Internal_GetTexturePages(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTexturePages(IntPtr thisPtr, RRef<Texture>[] value);
	}

	/** @} */
}

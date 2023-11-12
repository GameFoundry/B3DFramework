//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Implementation of SpriteImage that renders a single glyph from a Font.</summary>
	[ShowInInspector]
	public partial class SpriteGlyph : SpriteImage
	{
		private SpriteGlyph(bool __dummy0) { }
		protected SpriteGlyph() { }

		/// <summary>Creates a new sprite glyph.</summary>
		public SpriteGlyph(RRef<Font> font, int glyph, float size = 8f)
		{
			Internal_Create(this, font, glyph, size);
		}

		/// <summary>Creates a new sprite glyph.</summary>
		public SpriteGlyph(SpriteGlyphCreateInformation createInformation)
		{
			Internal_Create0(this, ref createInformation);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<SpriteGlyph> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<SpriteGlyph>(SpriteGlyph x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		/// <summary>Sets the font to render the glyph from.</summary>
		public void SetFont(RRef<Font> font)
		{
			Internal_SetFont(mCachedPtr, font);
		}

		/// <summary>Sets the unicode code of the glyph to render.</summary>
		public void SetGlyph(int glyph)
		{
			Internal_SetGlyph(mCachedPtr, glyph);
		}

		/// <summary>Sets the size of the glyph in points.</summary>
		public void SetGlyphSize(float size)
		{
			Internal_SetGlyphSize(mCachedPtr, size);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<SpriteGlyph> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFont(IntPtr thisPtr, RRef<Font> font);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGlyph(IntPtr thisPtr, int glyph);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGlyphSize(IntPtr thisPtr, float size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(SpriteGlyph managedInstance, RRef<Font> font, int glyph, float size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(SpriteGlyph managedInstance, ref SpriteGlyphCreateInformation createInformation);
	}

	/** @} */
}

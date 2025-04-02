//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Text
	 *  @{
	 */

	/// <summary>
	/// Contains information about font characters rendered into one or multiple bitmaps, for specific font size.
	/// </summary>
	[ShowInInspector]
	public partial class FontBitmapInformation : ScriptObject
	{
		private FontBitmapInformation(bool __dummy0) { }
		protected FontBitmapInformation() { }

		/// <summary>Font size for which the bitmaps are rendered.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Size
		{
			get { return Internal_GetSize(mCachedPtr); }
			set { Internal_SetSize(mCachedPtr, value); }
		}

		/// <summary>Y offset to the baseline on which the characters are placed, in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float BaselineOffset
		{
			get { return Internal_GetBaselineOffset(mCachedPtr); }
			set { Internal_SetBaselineOffset(mCachedPtr, value); }
		}

		/// <summary>Height of a single line of the font, in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float LineHeight
		{
			get { return Internal_GetLineHeight(mCachedPtr); }
			set { Internal_SetLineHeight(mCachedPtr, value); }
		}

		/// <summary>Character to use when data for a character is missing.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CharacterInformation MissingGlyph
		{
			get
			{
				CharacterInformation temp;
				Internal_GetMissingGlyph(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetMissingGlyph(mCachedPtr, ref value); }
		}

		/// <summary>Width of a space in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float SpaceWidth
		{
			get { return Internal_GetSpaceWidth(mCachedPtr); }
			set { Internal_SetSpaceWidth(mCachedPtr, value); }
		}

		/// <summary>Returns a character description for the character with the specified Unicode key.</summary>
		public CharacterInformation GetCharacterInformation(int characterId)
		{
			CharacterInformation temp;
			Internal_GetCharacterInformation(mCachedPtr, characterId, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCharacterInformation(IntPtr thisPtr, int characterId, out CharacterInformation __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSize(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetBaselineOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBaselineOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetLineHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLineHeight(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetMissingGlyph(IntPtr thisPtr, out CharacterInformation __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMissingGlyph(IntPtr thisPtr, ref CharacterInformation value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSpaceWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpaceWidth(IntPtr thisPtr, float value);
	}

	/** @} */
}

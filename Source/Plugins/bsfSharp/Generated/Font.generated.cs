//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI_Engine
	 *  @{
	 */

	/// <summary>
	/// Font resource containing data about textual characters and how to render text. Contains one or multiple font  bitmaps, 
	/// each for a specific size.
	/// </summary>
	[ShowInInspector]
	public partial class Font : Resource
	{
		private Font(bool __dummy0) { }
		protected Font() { }

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Font> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<Font>(Font x)
		{ return Internal_GetRef(x.mCachedPtr); }

		/// <summary>Returns font bitmap for a specific font size.</summary>
		/// <param name="size">Size of the bitmap in points.</param>
		/// <returns>Bitmap object if it exists, false otherwise.</returns>
		public FontBitmap GetBitmap(uint size)
		{
			return Internal_getBitmap(mCachedPtr, size);
		}

		/// <summary>Finds the available font bitmap size closest to the provided size.</summary>
		/// <param name="size">Size of the bitmap in points.</param>
		/// <returns>Nearest available bitmap size.</returns>
		public int GetClosestSize(uint size)
		{
			return Internal_getClosestSize(mCachedPtr, size);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Font> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FontBitmap Internal_getBitmap(IntPtr thisPtr, uint size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getClosestSize(IntPtr thisPtr, uint size);
	}

	/** @} */
}

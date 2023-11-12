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

	/// <summary>Provides easy access to variety of icons for use in the engine.</summary>
	[ShowInInspector]
	public partial class StockIcons : ScriptObject
	{
		private StockIcons(bool __dummy0) { }
		protected StockIcons() { }

		/// <summary>Retrieves a particular stock icon.</summary>
		/// <param name="icon">Icon to retrieve.</param>
		/// <param name="size">Size of the icon in points.</param>
		public static RRef<SpriteImage> GetIcon(StockIcon icon, float size = 8f)
		{
			return Internal_GetIcon(icon, size);
		}

		/// <summary>Returns the unicode character corresponding to an icon.</summary>
		public static int GetUnicode(StockIcon icon)
		{
			return Internal_GetUnicode(icon);
		}

		/// <summary>Returns the font in which the provided icon is stored in.</summary>
		public static RRef<Font> GetFont(StockIcon icon)
		{
			return Internal_GetFont(icon);
		}

		/// <summary>Parses an icon name and returns the corresponding enum entry if found.</summary>
		public static StockIcon ParseIconName(string name)
		{
			return Internal_ParseIconName(name);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<SpriteImage> Internal_GetIcon(StockIcon icon, float size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetUnicode(StockIcon icon);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Font> Internal_GetFont(StockIcon icon);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern StockIcon Internal_ParseIconName(string name);
	}

	/** @} */
}

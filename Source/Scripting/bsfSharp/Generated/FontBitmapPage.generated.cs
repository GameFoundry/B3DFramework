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

	/// <summary>Information about a single page containing font bitmaps.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct FontBitmapPage
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static FontBitmapPage Default()
		{
			FontBitmapPage value = new FontBitmapPage();
			value.Texture = null;
			value.Type = FontBitmapPageType.Runtime;

			return value;
		}

		public RRef<Texture> Texture;
		public FontBitmapPageType Type;
	}

	/** @} */
}

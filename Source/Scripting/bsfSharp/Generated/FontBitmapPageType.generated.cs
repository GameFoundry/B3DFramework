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

	/// <summary>Available types of FontBitmapPage.</summary>
	public enum FontBitmapPageType
	{
		/// <summary>Glyphs in this page can be dynamically allocated at runtime and won&apos;t be saved.</summary>
		Runtime = 0,
		/// <summary>
		/// Glyphs in this page can be dynamically allocated at runtime and will be saved. Next time they are loaded they will 
		/// use the Loaded type.
		/// </summary>
		Baked = 1,
		/// <summary>Glyphs in this page can be read, but no new glyphs can be added to the page.</summary>
		Loaded = 2
	}

	/** @} */
}

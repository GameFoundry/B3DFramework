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

	/// <summary>Determines how is a font rendered into the bitmap texture.</summary>
	public enum FontRenderMode
	{
		Smooth = 0,
		Raster = 1,
		HintedSmooth = 2,
		HintedRaster = 3
	}

	/** @} */
#endif
}

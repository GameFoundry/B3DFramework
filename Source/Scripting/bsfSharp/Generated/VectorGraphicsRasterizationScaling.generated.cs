//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup VectorGraphics
	 *  @{
	 */

	/// <summary>Determines how to scale path canvas when rasterizing for a particular size.</summary>
	public enum VectorGraphicsRasterizationScaling
	{
		/// <summary>Canvas will stretch non-uniformly in both dimensions in order to cover the raster area fully.</summary>
		StretchToFit = 0,
		/// <summary>
		/// Canvas will scale uniformly until one dimension is aligned with the raster area. Remaining dimension might have empty 
		/// space, and canvas will be placed in the center of the raster dimension.
		/// </summary>
		ScaleToFit = 1,
		/// <summary>
		/// Canvas will scale uniformly until both dimensions are larger or aligned with the raster area. Remaining dimension 
		/// might be cropped.
		/// </summary>
		CropToFit = 2,
		/// <summary>
		/// Do not perform any scaling. Canvas size is ignored and path coordinates are mapped 1:1 to raster coordinates.
		/// </summary>
		None = 3
	}

	/** @} */
}

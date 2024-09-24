//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>Type of scaling modes for GUI images.</summary>
	public enum TextureScaleMode
	{
		/// <summary>Image will keep its original size, but will repeat in order to fill the assigned area.</summary>
		RepeatToFit = 3,
		/// <summary>Image will stretch non-uniformly in all dimensions in order to cover the assigned area fully.</summary>
		StretchToFit = 0,
		/// <summary>
		/// Image will scale uniformly until one dimension is aligned with the assigned area. Remaining dimension might have 
		/// empty space.
		/// </summary>
		ScaleToFit = 1,
		/// <summary>
		/// Image will scale uniformly until both dimensions are larger or aligned with the assigned area. Remaining dimension 
		/// might be cropped.
		/// </summary>
		CropToFit = 2
	}

	/** @} */
}

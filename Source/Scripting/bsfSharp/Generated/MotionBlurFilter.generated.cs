//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Type of filter to use when filtering samples contributing to a blurred pixel.</summary>
	public enum MotionBlurFilter
	{
		/// <summary>Samples will be simply averaged together to create the blurred pixel.</summary>
		Simple = 0,
		/// <summary>
		/// A more advanced reconstruction filter will be used. This filter provides better blur quality at a performance cost. 
		/// In particular the filter will improve blurring at object boundaries, allowing blur to extend beyond the object 
		/// silhouette. It will also try to estimate blurred background and provide better weighting between background, center 
		/// and foreground samples.
		/// </summary>
		Reconstruction = 1
	}

	/** @} */
}

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

	/// <summary>Types of available depth of field effects.</summary>
	public enum DepthOfFieldType
	{
		/// <summary>
		/// Fastest depth of field variant, uses gaussian blur to approximate depth of field on near and far objects, but 
		/// doesn&apos;t use any physically based methods for determining blur amount.
		/// </summary>
		Gaussian = 0,
		/// <summary>
		/// Very expensive depth of field variant that allows you to use a bokeh texture, controlling the shape of the blur 
		/// (usually intended to mimic real world camera aperature shapes). Blur is varied according to actual object distance 
		/// and the effect is more physically based than gaussian blur (but not completely). Very expensive in terms of 
		/// performance.
		/// </summary>
		Bokeh = 1
	}

	/** @} */
}

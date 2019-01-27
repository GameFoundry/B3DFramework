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

	/// <summary>Probe type that determines the shape of the probe and how is it interpreted by the renderer.</summary>
	public enum ReflectionProbeType
	{
		/// <summary>
		/// Reflection probe cubemap is generated, and box extents are used for calculating influence ranges and box  geometry.
		/// </summary>
		Box = 0,
		/// <summary>
		/// Reflection probe cubemap is generated, but sphere is used for calculating the influence radius and proxy geometry.
		/// </summary>
		Sphere = 1
	}

	/** @} */
}

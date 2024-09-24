//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Determines how to sort particles before rendering.</summary>
	public enum ParticleSortMode
	{
		/// <summary>Sort by age, youngest to oldest.</summary>
		YoungToOld = 3,
		/// <summary>Do not sort the particles.</summary>
		None = 0,
		/// <summary>Sort by distance from the camera, furthest to nearest.</summary>
		Distance = 1,
		/// <summary>Sort by age, oldest to youngest.</summary>
		OldToYoung = 2
	}

	/** @} */
}

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
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Possible orientations when rendering billboard particles.</summary>
	public enum ParticleOrientation
	{
		/// <summary>Orient towards view (camera) plane.</summary>
		ViewPlane = 0,
		/// <summary>Orient towards view (camera) position.</summary>
		ViewPosition = 1,
		/// <summary>Orient with a user-provided axis.</summary>
		Plane = 2
	}

	/** @} */
}

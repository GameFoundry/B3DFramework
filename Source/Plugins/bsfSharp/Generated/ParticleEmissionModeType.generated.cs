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

	/// <summary>Types of emission modes.</summary>
	public enum ParticleEmissionModeType
	{
		/// <summary>Position will be picked randomly on a shape.</summary>
		Random = 0,
		/// <summary>Positions will loop around the shape in a predictable fashion.</summary>
		Loop = 1,
		/// <summary>Similar to Loop, except the order will be reversed when one loop iteration finishes.</summary>
		PingPong = 2,
		/// <summary>
		/// All particles spawned on the shape at some instant (usually a frame) will be spread around the shape equally.
		/// </summary>
		Spread = 3
	}

	/** @} */
}

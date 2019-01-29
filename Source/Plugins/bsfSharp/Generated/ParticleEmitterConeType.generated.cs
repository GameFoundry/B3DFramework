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

	/// <summary>Determines the emission type for the cone particle emitter shape.</summary>
	public enum ParticleEmitterConeType
	{
		/// <summary>Emit particles only from the cone base.</summary>
		Base = 0,
		/// <summary>Emit particles from the entire cone volume.</summary>
		Volume = 1
	}

	/** @} */
}

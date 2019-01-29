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
	public enum ParticleEmitterBoxType
	{
		/// <summary>Particles will be emitted from the entire volume.</summary>
		Volume = 0,
		/// <summary>Particles will be emitted only from box surface.</summary>
		Surface = 1,
		/// <summary>Particles will be emitted only from box edge.</summary>
		Edge = 2
	}

	/** @} */
}

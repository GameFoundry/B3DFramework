//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Determines how are particles represented on the screen.</summary>
	public enum ParticleRenderMode
	{
		/// <summary>Particle is represented using a 2D quad.</summary>
		Billboard = 0,
		/// <summary>Particle is represented using a 3D mesh.</summary>
		Mesh = 1
	}

	/** @} */
}

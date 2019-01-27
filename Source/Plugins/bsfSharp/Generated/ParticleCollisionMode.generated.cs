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

	/// <summary>Types of collision modes that ParticleCollisions evolver can operate in.</summary>
	public enum ParticleCollisionMode
	{
		/// <summary>Particles will collide with a user-provided set of planes.</summary>
		Plane = 0,
		/// <summary>Particles will collide with physics colliders in the scene.</summary>
		World = 1
	}

	/** @} */
}

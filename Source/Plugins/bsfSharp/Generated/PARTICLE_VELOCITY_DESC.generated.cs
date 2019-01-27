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

	/// <summary>Structure used for initializing a ParticleVelocity object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleVelocityOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleVelocityOptions Default()
		{
			ParticleVelocityOptions value = new ParticleVelocityOptions();
			value.velocity = null;
			value.worldSpace = false;

			return value;
		}

		/// <summary>Determines the velocity of the particles evaluated over particle lifetime.</summary>
		public Vector3Distribution velocity;
		/// <summary>True if the velocity is provided in world space, false if in local space.</summary>
		public bool worldSpace;
	}

	/** @} */
}

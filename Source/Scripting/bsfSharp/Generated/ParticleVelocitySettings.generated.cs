//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Structure used for initializing a ParticleVelocity object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleVelocitySettings
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleVelocitySettings Default()
		{
			ParticleVelocitySettings value = new ParticleVelocitySettings();
			value.Velocity = null;
			value.WorldSpace = false;

			return value;
		}

		/// <summary>Determines the velocity of the particles evaluated over particle lifetime.</summary>
		public Vector3Distribution Velocity;
		/// <summary>True if the velocity is provided in world space, false if in local space.</summary>
		public bool WorldSpace;
	}

	/** @} */
}

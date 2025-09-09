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

	/// <summary>Structure used for initializing a ParticleGravity object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleGravitySettings
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleGravitySettings Default()
		{
			ParticleGravitySettings value = new ParticleGravitySettings();
			value.Scale = 1f;

			return value;
		}

		/// <summary>Scale which to apply to the gravity value retrieved from the physics sub-system.</summary>
		public float Scale;
	}

	/** @} */
}

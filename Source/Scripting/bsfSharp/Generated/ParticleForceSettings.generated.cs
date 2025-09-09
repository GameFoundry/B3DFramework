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

	/// <summary>Structure used for initializing a ParticleForce object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleForceSettings
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleForceSettings Default()
		{
			ParticleForceSettings value = new ParticleForceSettings();
			value.Force = null;
			value.WorldSpace = false;

			return value;
		}

		/// <summary>Determines the force of the particles evaluated over particle lifetime.</summary>
		public Vector3Distribution Force;
		/// <summary>True if the force is provided in world space, false if in local space.</summary>
		public bool WorldSpace;
	}

	/** @} */
}

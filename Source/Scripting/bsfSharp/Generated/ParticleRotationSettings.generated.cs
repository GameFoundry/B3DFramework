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

	/// <summary>Structure used for initializing a ParticleRotation object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleRotationSettings
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleRotationSettings Default()
		{
			ParticleRotationSettings value = new ParticleRotationSettings();
			value.Rotation = new FloatDistribution(0f);
			value.Rotation3D = null;
			value.Use3DRotation = false;

			return value;
		}

		/// <summary>
		/// Determines the rotation of the particles in degrees, applied around the particle&apos;s local Z axis. Only used if 3D 
		/// rotation is disabled.
		/// </summary>
		public FloatDistribution Rotation;
		/// <summary>
		/// Determines the rotation of the particles in degrees as Euler angles. Only used if 3D rotation is enabled.
		/// </summary>
		public Vector3Distribution Rotation3D;
		/// <summary>
		/// Determines should the particle rotation be a single angle applied around a Z axis (if disabled), or a set of Euler 
		/// angles that allow you to rotate around every axis (if enabled).
		/// </summary>
		public bool Use3DRotation;
	}

	/** @} */
}

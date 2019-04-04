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

	/// <summary>Structure used for initializing a ParticleRotation object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleRotationOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleRotationOptions Default()
		{
			ParticleRotationOptions value = new ParticleRotationOptions();
			value.rotation = new FloatDistribution(0f);
			value.rotation3D = null;
			value.use3DRotation = false;

			return value;
		}

		/// <summary>
		/// Determines the rotation of the particles in degrees, applied around the particle&apos;s local Z axis. Only used if  
		/// 3D rotation is disabled.
		/// </summary>
		public FloatDistribution rotation;
		/// <summary>
		/// Determines the rotation of the particles in degrees as Euler angles. Only used if 3D rotation is enabled.
		/// </summary>
		public Vector3Distribution rotation3D;
		/// <summary>
		/// Determines should the particle rotation be a single angle applied around a Z axis (if disabled), or a  set of Euler 
		/// angles that allow you to rotate around every axis (if enabled).
		/// </summary>
		public bool use3DRotation;
	}

	/** @} */
}

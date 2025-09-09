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

	/// <summary>Structure used for initializing a ParticleOrbit object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleOrbitSettings
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleOrbitSettings Default()
		{
			ParticleOrbitSettings value = new ParticleOrbitSettings();
			value.Center = null;
			value.Velocity = null;
			value.Radial = new FloatDistribution(0f);
			value.WorldSpace = false;

			return value;
		}

		/// <summary>Position of the center around which to orbit. Evaluated over particle system lifetime.</summary>
		public Vector3Distribution Center;
		/// <summary>
		/// Determines the speed of rotation around each axis. The speed is specified in &quot;turns&quot; where 0 = no rotation, 
		/// 0.5 = 180 degree rotation and 1 = 360 degree rotation. Evaluated over particle lifetime.
		/// </summary>
		public Vector3Distribution Velocity;
		/// <summary>
		/// Speed at which to push or pull the particles towards/away from the center. Evaluated over particle lifetime.
		/// </summary>
		public FloatDistribution Radial;
		/// <summary>True if the properties provided are in world space, false if in local space.</summary>
		public bool WorldSpace;
	}

	/** @} */
}

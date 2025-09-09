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

	/// <summary>Information describing a ParticleEmitterSphereShape.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleSphereShapeSettings
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleSphereShapeSettings Default()
		{
			ParticleSphereShapeSettings value = new ParticleSphereShapeSettings();
			value.Radius = 1f;
			value.Thickness = 0f;

			return value;
		}

		/// <summary>Radius of the sphere.</summary>
		public float Radius;
		/// <summary>
		/// Proportion of the volume that can emit particles. Thickness of 0 results in particles being emitted only from the 
		/// edge of the volume, while thickness of 1 results in particles being emitted from the entire volume. In-between values 
		/// will use a part of the volume.
		/// </summary>
		public float Thickness;
	}

	/** @} */
}

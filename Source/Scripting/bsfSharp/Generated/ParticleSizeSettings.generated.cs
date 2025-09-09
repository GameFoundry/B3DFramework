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

	/// <summary>Structure used for initializing a ParticleSize object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleSizeSettings
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleSizeSettings Default()
		{
			ParticleSizeSettings value = new ParticleSizeSettings();
			value.Size = new FloatDistribution(1f);
			value.Size3D = null;
			value.Use3DSize = false;

			return value;
		}

		/// <summary>
		/// Determines the uniform size of the particles evaluated over particle lifetime. Only used if 3D size is disabled.
		/// </summary>
		public FloatDistribution Size;
		/// <summary>
		/// Determines the non-uniform size of the particles evaluated over particle lifetime. Only used if 3D size is enabled.
		/// </summary>
		public Vector3Distribution Size3D;
		/// <summary>
		/// Determines should the size be evaluated uniformly for all dimensions, or evaluate each dimension with its own 
		/// distribution.
		/// </summary>
		public bool Use3DSize;
	}

	/** @} */
}

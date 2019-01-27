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

	/// <summary>Structure used for initializing a ParticleSize object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleSizeOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleSizeOptions Default()
		{
			ParticleSizeOptions value = new ParticleSizeOptions();
			value.size = new FloatDistribution(1f);
			value.size3D = null;
			value.use3DSize = false;

			return value;
		}

		/// <summary>
		/// Determines the uniform size of the particles evaluated over particle lifetime. Only used if 3D size is disabled.
		/// </summary>
		public FloatDistribution size;
		/// <summary>
		/// Determines the non-uniform size of the particles evaluated over particle lifetime. Only used if 3D size is enabled.
		/// </summary>
		public Vector3Distribution size3D;
		/// <summary>
		/// Determines should the size be evaluated uniformly for all dimensions, or evaluate each dimension with its own 
		/// distribution.
		/// </summary>
		public bool use3DSize;
	}

	/** @} */
}

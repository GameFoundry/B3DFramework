//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Structure used for initializing a ParticleGravity object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleGravityOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleGravityOptions Default()
		{
			ParticleGravityOptions value = new ParticleGravityOptions();
			value.scale = 1f;

			return value;
		}

		/// <summary>Scale which to apply to the gravity value retrieved from the physics sub-system.</summary>
		public float scale;
	}

	/** @} */
}

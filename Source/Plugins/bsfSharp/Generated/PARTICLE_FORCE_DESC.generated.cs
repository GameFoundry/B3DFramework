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

	/// <summary>Structure used for initializing a ParticleForce object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleForceOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleForceOptions Default()
		{
			ParticleForceOptions value = new ParticleForceOptions();
			value.force = null;
			value.worldSpace = false;

			return value;
		}

		/// <summary>Determines the force of the particles evaluated over particle lifetime.</summary>
		public Vector3Distribution force;
		/// <summary>True if the force is provided in world space, false if in local space.</summary>
		public bool worldSpace;
	}

	/** @} */
}

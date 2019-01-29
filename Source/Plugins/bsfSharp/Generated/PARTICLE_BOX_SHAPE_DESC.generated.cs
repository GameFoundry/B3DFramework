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

	/// <summary>Information describing a ParticleEmitterBoxShape.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleBoxShapeOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleBoxShapeOptions Default()
		{
			ParticleBoxShapeOptions value = new ParticleBoxShapeOptions();
			value.type = ParticleEmitterBoxType.Volume;
			value.extents = new Vector3();

			return value;
		}

		/// <summary>Determines from which portion of the box should particles be emitted from.</summary>
		public ParticleEmitterBoxType type;
		/// <summary>Extends of the box.</summary>
		public Vector3 extents;
	}

	/** @} */
}

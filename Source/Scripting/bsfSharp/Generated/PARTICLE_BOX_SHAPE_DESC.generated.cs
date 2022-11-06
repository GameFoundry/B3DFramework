//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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
			value.Type = ParticleEmitterBoxType.Volume;
			value.Extents = Vector3.Default();

			return value;
		}

		/// <summary>Determines from which portion of the box should particles be emitted from.</summary>
		public ParticleEmitterBoxType Type;
		/// <summary>Extends of the box.</summary>
		public Vector3 Extents;
	}

	/** @} */
}

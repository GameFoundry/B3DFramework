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

	/// <summary>Information describing a ParticleEmitterLineShape.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleLineShapeOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleLineShapeOptions Default()
		{
			ParticleLineShapeOptions value = new ParticleLineShapeOptions();
			value.Length = 1f;
			value.Mode = ParticleEmissionMode.Default();

			return value;
		}

		/// <summary>Length of the line.</summary>
		public float Length;
		/// <summary>Determines how will particle positions on the shape be generated.</summary>
		public ParticleEmissionMode Mode;
	}

	/** @} */
}

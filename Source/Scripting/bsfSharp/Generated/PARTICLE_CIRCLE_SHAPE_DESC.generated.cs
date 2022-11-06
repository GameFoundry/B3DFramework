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

	/// <summary>Information describing a ParticleEmitterCircleShape.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleCircleShapeOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleCircleShapeOptions Default()
		{
			ParticleCircleShapeOptions value = new ParticleCircleShapeOptions();
			value.Radius = 1f;
			value.Thickness = 0f;
			value.Arc = Degree.Default();
			value.Mode = ParticleEmissionMode.Default();

			return value;
		}

		/// <summary>Radius of the circle.</summary>
		public float Radius;
		/// <summary>
		/// Proportion of the surface that can emit particles. Thickness of 0 results in particles being emitted only from the 
		/// edge of the circle, while thickness of 1 results in particles being emitted from the entire surface. In-between 
		/// values will use a part of the surface.
		/// </summary>
		public float Thickness;
		/// <summary>Angular portion of the cone from which to emit particles from, in degrees.</summary>
		public Degree Arc;
		/// <summary>Determines how will particle positions on the shape be generated.</summary>
		public ParticleEmissionMode Mode;
	}

	/** @} */
}

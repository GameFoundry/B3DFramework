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

	/// <summary>Information describing a ParticleEmitterConeShape.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleConeShapeOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleConeShapeOptions Default()
		{
			ParticleConeShapeOptions value = new ParticleConeShapeOptions();
			value.Type = ParticleEmitterConeType.Base;
			value.Radius = 0f;
			value.Angle = Degree.Default();
			value.Length = 1f;
			value.Thickness = 1f;
			value.Arc = Degree.Default();
			value.Mode = ParticleEmissionMode.Default();

			return value;
		}

		/// <summary>Determines where on the cone are the particles emitter from.</summary>
		public ParticleEmitterConeType Type;
		/// <summary>Radius of the cone base.</summary>
		public float Radius;
		/// <summary>Angle of the cone.</summary>
		public Degree Angle;
		/// <summary>Length of the cone. Irrelevant if emission type is Base.</summary>
		public float Length;
		/// <summary>
		/// Proportion of the volume that can emit particles. Thickness of 0 results in particles being emitted only from the 
		/// edge of the cone, while thickness of 1 results in particles being emitted from the entire volume. In-between values 
		/// will use a part of the volume.
		/// </summary>
		public float Thickness;
		/// <summary>Angular portion of the cone from which to emit particles from, in degrees.</summary>
		public Degree Arc;
		/// <summary>Determines how will particle positions on the shape be generated.</summary>
		public ParticleEmissionMode Mode;
	}

	/** @} */
}

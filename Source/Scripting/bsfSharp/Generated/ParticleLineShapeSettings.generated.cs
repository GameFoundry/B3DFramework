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

	/// <summary>Information describing a ParticleEmitterLineShape.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleLineShapeSettings
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleLineShapeSettings Default()
		{
			ParticleLineShapeSettings value = new ParticleLineShapeSettings();
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

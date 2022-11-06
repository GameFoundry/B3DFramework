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

	/// <summary>Structure used for initializing a ParticleTextureAnimation object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleTextureAnimationOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleTextureAnimationOptions Default()
		{
			ParticleTextureAnimationOptions value = new ParticleTextureAnimationOptions();
			value.RandomizeRow = false;
			value.NumCycles = 1;

			return value;
		}

		/// <summary>
		/// Randomly pick a row to use for animation when the particle is first spawned. This implies that only a single row of 
		/// the grid will be used for individual particle&apos;s animation.
		/// </summary>
		public bool RandomizeRow;
		/// <summary>Number of cycles to loop the animation during particle&apos;s lifetime.</summary>
		public int NumCycles;
	}

	/** @} */
}

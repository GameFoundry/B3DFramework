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

	/// <summary>
	/// Provides functionality for particle texture animation. Uses the sprite texture assigned to the particle&apos;s 
	/// material to determine animation properties.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleTextureAnimation : ParticleEvolver
	{
		private ParticleTextureAnimation(bool __dummy0) { }

		/// <summary>Creates a new particle texture animation evolver.</summary>
		public ParticleTextureAnimation(ParticleTextureAnimationSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle texture animation evolver.</summary>
		public ParticleTextureAnimation()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleTextureAnimationSettings Settings
		{
			get
			{
				ParticleTextureAnimationSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleTextureAnimationSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleTextureAnimationSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleTextureAnimation managedInstance, ref ParticleTextureAnimationSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleTextureAnimation managedInstance);
	}

	/** @} */
}

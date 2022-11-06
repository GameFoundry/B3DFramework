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

	/// <summary>
	/// Provides functionality for particle texture animation. Uses the sprite texture assigned to the particle&apos;s 
	/// material to determine animation properties.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleTextureAnimation : ParticleEvolver
	{
		private ParticleTextureAnimation(bool __dummy0) { }

		/// <summary>Creates a new particle texture animation evolver.</summary>
		public ParticleTextureAnimation(ParticleTextureAnimationOptions desc)
		{
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle texture animation evolver.</summary>
		public ParticleTextureAnimation()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleTextureAnimationOptions Options
		{
			get
			{
				ParticleTextureAnimationOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleTextureAnimationOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleTextureAnimationOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleTextureAnimation managedInstance, ref ParticleTextureAnimationOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleTextureAnimation managedInstance);
	}

	/** @} */
}

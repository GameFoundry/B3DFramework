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

	/// <summary>Changes the size of the particles over the particle lifetime.</summary>
	[ShowInInspector]
	public partial class ParticleSize : ParticleEvolver
	{
		private ParticleSize(bool __dummy0) { }

		/// <summary>Creates a new particle size evolver.</summary>
		public ParticleSize(ParticleSizeSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle size evolver.</summary>
		public ParticleSize()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleSizeSettings Settings
		{
			get
			{
				ParticleSizeSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleSizeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleSizeSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleSize managedInstance, ref ParticleSizeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleSize managedInstance);
	}

	/** @} */
}

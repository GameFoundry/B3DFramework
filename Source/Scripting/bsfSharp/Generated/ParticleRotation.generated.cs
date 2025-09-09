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

	/// <summary>Rotates the particles over the particle lifetime.</summary>
	[ShowInInspector]
	public partial class ParticleRotation : ParticleEvolver
	{
		private ParticleRotation(bool __dummy0) { }

		/// <summary>Creates a new particle rotation evolver.</summary>
		public ParticleRotation(ParticleRotationSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle rotation evolver.</summary>
		public ParticleRotation()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleRotationSettings Settings
		{
			get
			{
				ParticleRotationSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleRotationSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleRotationSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleRotation managedInstance, ref ParticleRotationSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleRotation managedInstance);
	}

	/** @} */
}

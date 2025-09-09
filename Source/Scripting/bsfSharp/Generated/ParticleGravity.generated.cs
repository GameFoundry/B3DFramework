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

	/// <summary>Applies gravity to the particles.</summary>
	[ShowInInspector]
	public partial class ParticleGravity : ParticleEvolver
	{
		private ParticleGravity(bool __dummy0) { }

		/// <summary>Creates a new particle gravity evolver.</summary>
		public ParticleGravity(ParticleGravitySettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle gravity evolver.</summary>
		public ParticleGravity()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleGravitySettings Settings
		{
			get
			{
				ParticleGravitySettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleGravitySettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleGravitySettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleGravity managedInstance, ref ParticleGravitySettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleGravity managedInstance);
	}

	/** @} */
}

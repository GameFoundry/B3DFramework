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

	/// <summary>Applies an arbitrary force to the particles.</summary>
	[ShowInInspector]
	public partial class ParticleForce : ParticleEvolver
	{
		private ParticleForce(bool __dummy0) { }

		/// <summary>Creates a new particle force evolver.</summary>
		public ParticleForce(ParticleForceSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle force evolver.</summary>
		public ParticleForce()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleForceSettings Settings
		{
			get
			{
				ParticleForceSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleForceSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleForceSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleForce managedInstance, ref ParticleForceSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleForce managedInstance);
	}

	/** @} */
}

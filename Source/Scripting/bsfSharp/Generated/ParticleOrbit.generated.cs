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
	/// Moves particles so that their sprites orbit their center according to the provided offset and rotation values.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleOrbit : ParticleEvolver
	{
		private ParticleOrbit(bool __dummy0) { }

		/// <summary>Creates a new particle orbit evolver.</summary>
		public ParticleOrbit(ParticleOrbitSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle orbit evolver.</summary>
		public ParticleOrbit()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleOrbitSettings Settings
		{
			get
			{
				ParticleOrbitSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleOrbitSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleOrbitSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleOrbit managedInstance, ref ParticleOrbitSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleOrbit managedInstance);
	}

	/** @} */
}

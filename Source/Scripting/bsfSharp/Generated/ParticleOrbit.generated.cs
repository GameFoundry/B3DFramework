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
	/// Moves particles so that their sprites orbit their center according to the provided offset and rotation values.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleOrbit : ParticleEvolver
	{
		private ParticleOrbit(bool __dummy0) { }

		/// <summary>Creates a new particle orbit evolver.</summary>
		public ParticleOrbit(ParticleOrbitOptions desc)
		{
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle orbit evolver.</summary>
		public ParticleOrbit()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleOrbitOptions Options
		{
			get
			{
				ParticleOrbitOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleOrbitOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleOrbitOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleOrbit managedInstance, ref ParticleOrbitOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleOrbit managedInstance);
	}

	/** @} */
}

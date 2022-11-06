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

	/// <summary>Applies an arbitrary force to the particles.</summary>
	[ShowInInspector]
	public partial class ParticleForce : ParticleEvolver
	{
		private ParticleForce(bool __dummy0) { }

		/// <summary>Creates a new particle force evolver.</summary>
		public ParticleForce(ParticleForceOptions desc)
		{
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle force evolver.</summary>
		public ParticleForce()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleForceOptions Options
		{
			get
			{
				ParticleForceOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleForceOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleForceOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleForce managedInstance, ref ParticleForceOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleForce managedInstance);
	}

	/** @} */
}

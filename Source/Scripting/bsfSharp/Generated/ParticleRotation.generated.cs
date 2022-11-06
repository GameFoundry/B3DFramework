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

	/// <summary>Rotates the particles over the particle lifetime.</summary>
	[ShowInInspector]
	public partial class ParticleRotation : ParticleEvolver
	{
		private ParticleRotation(bool __dummy0) { }

		/// <summary>Creates a new particle rotation evolver.</summary>
		public ParticleRotation(ParticleRotationOptions desc)
		{
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle rotation evolver.</summary>
		public ParticleRotation()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleRotationOptions Options
		{
			get
			{
				ParticleRotationOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleRotationOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleRotationOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleRotation managedInstance, ref ParticleRotationOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleRotation managedInstance);
	}

	/** @} */
}

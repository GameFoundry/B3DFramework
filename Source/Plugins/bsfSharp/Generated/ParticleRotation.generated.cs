//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle rotation evolver.</summary>
		public ParticleRotation()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleRotationOptions Options
		{
			get
			{
				ParticleRotationOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleRotationOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleRotationOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleRotation managedInstance, ref ParticleRotationOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleRotation managedInstance);
	}

	/** @} */
}

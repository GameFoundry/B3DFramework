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

	/// <summary>Applies linear velocity to the particles.</summary>
	[ShowInInspector]
	public partial class ParticleVelocity : ParticleEvolver
	{
		private ParticleVelocity(bool __dummy0) { }

		/// <summary>Creates a new particle velocity evolver.</summary>
		public ParticleVelocity(ParticleVelocityOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle velocity evolver.</summary>
		public ParticleVelocity()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleVelocityOptions Options
		{
			get
			{
				ParticleVelocityOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleVelocityOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleVelocityOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleVelocity managedInstance, ref ParticleVelocityOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleVelocity managedInstance);
	}

	/** @} */
}

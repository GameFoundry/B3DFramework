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

	/// <summary>Applies gravity to the particles.</summary>
	[ShowInInspector]
	public partial class ParticleGravity : ParticleEvolver
	{
		private ParticleGravity(bool __dummy0) { }

		/// <summary>Creates a new particle gravity evolver.</summary>
		public ParticleGravity(ParticleGravityOptions desc)
		{
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle gravity evolver.</summary>
		public ParticleGravity()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleGravityOptions Options
		{
			get
			{
				ParticleGravityOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleGravityOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleGravityOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleGravity managedInstance, ref ParticleGravityOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleGravity managedInstance);
	}

	/** @} */
}

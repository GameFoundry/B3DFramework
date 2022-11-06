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

	/// <summary>Changes the size of the particles over the particle lifetime.</summary>
	[ShowInInspector]
	public partial class ParticleSize : ParticleEvolver
	{
		private ParticleSize(bool __dummy0) { }

		/// <summary>Creates a new particle size evolver.</summary>
		public ParticleSize(ParticleSizeOptions desc)
		{
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle size evolver.</summary>
		public ParticleSize()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleSizeOptions Options
		{
			get
			{
				ParticleSizeOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleSizeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleSizeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleSize managedInstance, ref ParticleSizeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleSize managedInstance);
	}

	/** @} */
}

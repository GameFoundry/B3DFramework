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

	/// <summary>Changes the size of the particles over the particle lifetime.</summary>
	[ShowInInspector]
	public partial class ParticleSize : ParticleEvolver
	{
		private ParticleSize(bool __dummy0) { }

		/// <summary>Creates a new particle size evolver.</summary>
		public ParticleSize(ParticleSizeOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle size evolver.</summary>
		public ParticleSize()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleSizeOptions Options
		{
			get
			{
				ParticleSizeOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleSizeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleSizeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleSize managedInstance, ref ParticleSizeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleSize managedInstance);
	}

	/** @} */
}

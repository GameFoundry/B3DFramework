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
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle orbit evolver.</summary>
		public ParticleOrbit()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleOrbitOptions Options
		{
			get
			{
				ParticleOrbitOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleOrbitOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleOrbitOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleOrbit managedInstance, ref ParticleOrbitOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleOrbit managedInstance);
	}

	/** @} */
}

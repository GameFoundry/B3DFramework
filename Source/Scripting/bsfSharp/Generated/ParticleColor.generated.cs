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

	/// <summary>Changes the color of the particles over the particle lifetime.</summary>
	[ShowInInspector]
	public partial class ParticleColor : ParticleEvolver
	{
		private ParticleColor(bool __dummy0) { }

		/// <summary>Creates a new particle color evolver.</summary>
		public ParticleColor(ParticleColorOptions settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle color evolver.</summary>
		public ParticleColor()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleColorOptions Settings
		{
			get
			{
				ParticleColorOptions temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleColorOptions settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleColorOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleColor managedInstance, ref ParticleColorOptions settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleColor managedInstance);
	}

	/** @} */
}

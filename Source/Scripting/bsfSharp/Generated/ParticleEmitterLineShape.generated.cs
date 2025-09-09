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

	/// <summary>Particle emitter shape that emits particles from a line segment.</summary>
	[ShowInInspector]
	public partial class ParticleEmitterLineShape : ParticleEmitterShape
	{
		private ParticleEmitterLineShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter edge shape.</summary>
		public ParticleEmitterLineShape(ParticleLineShapeSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle emitter edge shape.</summary>
		public ParticleEmitterLineShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleLineShapeSettings Settings
		{
			get
			{
				ParticleLineShapeSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleLineShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleLineShapeSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterLineShape managedInstance, ref ParticleLineShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterLineShape managedInstance);
	}

	/** @} */
}

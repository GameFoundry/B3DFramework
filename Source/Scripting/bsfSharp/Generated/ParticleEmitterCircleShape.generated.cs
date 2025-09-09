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

	/// <summary>
	/// Particle emitter shape that emits particles from a circle. Using the thickness parameter you can control whether to 
	/// emit only from circle edge, the entire surface or just a part of the surface. Using the arc parameter you can emit 
	/// from a specific angular portion of the circle.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterCircleShape : ParticleEmitterShape
	{
		private ParticleEmitterCircleShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter circle shape.</summary>
		public ParticleEmitterCircleShape(ParticleCircleShapeSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle emitter circle shape.</summary>
		public ParticleEmitterCircleShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleCircleShapeSettings Settings
		{
			get
			{
				ParticleCircleShapeSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleCircleShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleCircleShapeSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterCircleShape managedInstance, ref ParticleCircleShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterCircleShape managedInstance);
	}

	/** @} */
}

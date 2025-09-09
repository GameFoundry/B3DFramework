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
	/// Particle emitter shape that emits particles from an axis aligned box. Particles can be emitted from box volume, 
	/// surface or edges. All particles have their normals set to positive Z direction.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterBoxShape : ParticleEmitterShape
	{
		private ParticleEmitterBoxShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter box shape.</summary>
		public ParticleEmitterBoxShape(ParticleBoxShapeSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle emitter box shape.</summary>
		public ParticleEmitterBoxShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleBoxShapeSettings Settings
		{
			get
			{
				ParticleBoxShapeSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleBoxShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleBoxShapeSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterBoxShape managedInstance, ref ParticleBoxShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterBoxShape managedInstance);
	}

	/** @} */
}

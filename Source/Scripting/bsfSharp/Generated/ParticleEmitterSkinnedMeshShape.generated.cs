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
	/// Particle emitter shape that emits particles from a surface of a skinned (animated) mesh. Particles can be emitted from 
	/// mesh vertices, edges or triangles. If information about normals exists, particles will also inherit the normals.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterSkinnedMeshShape : ParticleEmitterShape
	{
		private ParticleEmitterSkinnedMeshShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter skinned mesh shape.</summary>
		public ParticleEmitterSkinnedMeshShape(ParticleSkinnedMeshShapeSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle emitter skinned mesh shape.</summary>
		public ParticleEmitterSkinnedMeshShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleSkinnedMeshShapeSettings Settings
		{
			get
			{
				ParticleSkinnedMeshShapeSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleSkinnedMeshShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleSkinnedMeshShapeSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterSkinnedMeshShape managedInstance, ref ParticleSkinnedMeshShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterSkinnedMeshShape managedInstance);
	}

	/** @} */
}

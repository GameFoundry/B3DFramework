//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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
		public ParticleEmitterSkinnedMeshShape(ParticleSkinnedMeshShapeOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter skinned mesh shape.</summary>
		public ParticleEmitterSkinnedMeshShape()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleSkinnedMeshShapeOptions Options
		{
			get
			{
				ParticleSkinnedMeshShapeOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleSkinnedMeshShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleSkinnedMeshShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleEmitterSkinnedMeshShape managedInstance, ref ParticleSkinnedMeshShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleEmitterSkinnedMeshShape managedInstance);
	}

	/** @} */
}

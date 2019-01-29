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
	/// Particle emitter shape that emits particles from a surface of a static (non-animated) mesh. Particles can be emitted 
	/// from mesh vertices, edges or triangles. If information about normals exists, particles will also inherit the normals.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterStaticMeshShape : ParticleEmitterShape
	{
		private ParticleEmitterStaticMeshShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter static mesh shape.</summary>
		public ParticleEmitterStaticMeshShape(ParticleStaticMeshShapeOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter static mesh shape.</summary>
		public ParticleEmitterStaticMeshShape()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleStaticMeshShapeOptions Options
		{
			get
			{
				ParticleStaticMeshShapeOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleStaticMeshShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleStaticMeshShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleEmitterStaticMeshShape managedInstance, ref ParticleStaticMeshShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleEmitterStaticMeshShape managedInstance);
	}

	/** @} */
}

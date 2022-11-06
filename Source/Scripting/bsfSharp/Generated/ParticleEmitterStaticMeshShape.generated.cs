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
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter static mesh shape.</summary>
		public ParticleEmitterStaticMeshShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleStaticMeshShapeOptions Options
		{
			get
			{
				ParticleStaticMeshShapeOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleStaticMeshShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleStaticMeshShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterStaticMeshShape managedInstance, ref ParticleStaticMeshShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterStaticMeshShape managedInstance);
	}

	/** @} */
}

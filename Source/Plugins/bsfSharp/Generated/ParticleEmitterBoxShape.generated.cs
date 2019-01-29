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
	/// Particle emitter shape that emits particles from an axis aligned box. Particles can be emitted from box volume, 
	/// surface or edges. All particles have their normals set to positive Z direction.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterBoxShape : ParticleEmitterShape
	{
		private ParticleEmitterBoxShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter box shape.</summary>
		public ParticleEmitterBoxShape(ParticleBoxShapeOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter box shape.</summary>
		public ParticleEmitterBoxShape()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleBoxShapeOptions Options
		{
			get
			{
				ParticleBoxShapeOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleBoxShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleBoxShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleEmitterBoxShape managedInstance, ref ParticleBoxShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleEmitterBoxShape managedInstance);
	}

	/** @} */
}

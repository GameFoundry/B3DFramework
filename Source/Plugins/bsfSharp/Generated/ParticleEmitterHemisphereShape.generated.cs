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
	/// Particle emitter shape that emits particles from a hemisphere. Particles can be emitted from the hemisphere surface, 
	/// the entire volume or a proportion of the volume depending on the thickness parameter. All particles will have  normals 
	/// pointing outwards in a spherical direction.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterHemisphereShape : ParticleEmitterShape
	{
		private ParticleEmitterHemisphereShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter sphere shape.</summary>
		public ParticleEmitterHemisphereShape(ParticleHemisphereShapeOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter sphere shape.</summary>
		public ParticleEmitterHemisphereShape()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleHemisphereShapeOptions Options
		{
			get
			{
				ParticleHemisphereShapeOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleHemisphereShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleHemisphereShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleEmitterHemisphereShape managedInstance, ref ParticleHemisphereShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleEmitterHemisphereShape managedInstance);
	}

	/** @} */
}

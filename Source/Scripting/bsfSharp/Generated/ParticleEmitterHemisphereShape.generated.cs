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
	/// Particle emitter shape that emits particles from a hemisphere. Particles can be emitted from the hemisphere surface, 
	/// the entire volume or a proportion of the volume depending on the thickness parameter. All particles will have normals 
	/// pointing outwards in a spherical direction.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterHemisphereShape : ParticleEmitterShape
	{
		private ParticleEmitterHemisphereShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter sphere shape.</summary>
		public ParticleEmitterHemisphereShape(ParticleHemisphereShapeSettings settings)
		{
			Internal_Create(this, ref settings);
		}

		/// <summary>Creates a new particle emitter sphere shape.</summary>
		public ParticleEmitterHemisphereShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleHemisphereShapeSettings Settings
		{
			get
			{
				ParticleHemisphereShapeSettings temp;
				Internal_GetSettings(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSettings(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ref ParticleHemisphereShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSettings(IntPtr thisPtr, out ParticleHemisphereShapeSettings __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterHemisphereShape managedInstance, ref ParticleHemisphereShapeSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterHemisphereShape managedInstance);
	}

	/** @} */
}

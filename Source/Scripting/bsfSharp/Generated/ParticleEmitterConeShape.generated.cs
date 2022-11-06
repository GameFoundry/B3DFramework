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
	/// Particle emitter shape that emits particles from a cone. Particles can be created on cone base or volume, while 
	/// controling the radial arc of the emitted portion of the volume, as well as thickness of the cone emission volume. All 
	/// particles will have random normals within the distribution of the cone.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterConeShape : ParticleEmitterShape
	{
		private ParticleEmitterConeShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter cone shape.</summary>
		public ParticleEmitterConeShape(ParticleConeShapeOptions desc)
		{
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter cone shape.</summary>
		public ParticleEmitterConeShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleConeShapeOptions Options
		{
			get
			{
				ParticleConeShapeOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleConeShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleConeShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterConeShape managedInstance, ref ParticleConeShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterConeShape managedInstance);
	}

	/** @} */
}

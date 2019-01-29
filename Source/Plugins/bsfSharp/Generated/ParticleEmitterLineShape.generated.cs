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

	/// <summary>Particle emitter shape that emits particles from a line segment.</summary>
	[ShowInInspector]
	public partial class ParticleEmitterLineShape : ParticleEmitterShape
	{
		private ParticleEmitterLineShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter edge shape.</summary>
		public ParticleEmitterLineShape(ParticleLineShapeOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter edge shape.</summary>
		public ParticleEmitterLineShape()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleLineShapeOptions Options
		{
			get
			{
				ParticleLineShapeOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleLineShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleLineShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleEmitterLineShape managedInstance, ref ParticleLineShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleEmitterLineShape managedInstance);
	}

	/** @} */
}

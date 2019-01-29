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

	/// <summary>Particle emitter shape that emits particles from the surface of a rectangle.</summary>
	[ShowInInspector]
	public partial class ParticleEmitterRectShape : ParticleEmitterShape
	{
		private ParticleEmitterRectShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter rectangle shape.</summary>
		public ParticleEmitterRectShape(ParticleRectShapeOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter rectangle shape.</summary>
		public ParticleEmitterRectShape()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleRectShapeOptions Options
		{
			get
			{
				ParticleRectShapeOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleRectShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleRectShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleEmitterRectShape managedInstance, ref ParticleRectShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleEmitterRectShape managedInstance);
	}

	/** @} */
}

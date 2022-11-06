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

	/// <summary>Particle emitter shape that emits particles from the surface of a rectangle.</summary>
	[ShowInInspector]
	public partial class ParticleEmitterRectShape : ParticleEmitterShape
	{
		private ParticleEmitterRectShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter rectangle shape.</summary>
		public ParticleEmitterRectShape(ParticleRectShapeOptions desc)
		{
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter rectangle shape.</summary>
		public ParticleEmitterRectShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleRectShapeOptions Options
		{
			get
			{
				ParticleRectShapeOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleRectShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleRectShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterRectShape managedInstance, ref ParticleRectShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterRectShape managedInstance);
	}

	/** @} */
}

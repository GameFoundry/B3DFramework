//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	/// Particle emitter shape that emits particles from a circle. Using the thickness parameter you can control whether to 
	/// emit only from circle edge, the entire surface or just a part of the surface. Using the arc parameter you can emit 
	/// from a specific angular portion of the circle.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleEmitterCircleShape : ParticleEmitterShape
	{
		private ParticleEmitterCircleShape(bool __dummy0) { }

		/// <summary>Creates a new particle emitter circle shape.</summary>
		public ParticleEmitterCircleShape(ParticleCircleShapeOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter circle shape.</summary>
		public ParticleEmitterCircleShape()
		{
			Internal_create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleCircleShapeOptions Options
		{
			get
			{
				ParticleCircleShapeOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleCircleShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleCircleShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleEmitterCircleShape managedInstance, ref ParticleCircleShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleEmitterCircleShape managedInstance);
	}

	/** @} */
}

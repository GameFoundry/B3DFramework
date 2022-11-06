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
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle emitter box shape.</summary>
		public ParticleEmitterBoxShape()
		{
			Internal_Create0(this);
		}

		/// <summary>Options describing the shape.</summary>
		[ShowInInspector]
		[Inline]
		[NativeWrapper]
		public ParticleBoxShapeOptions Options
		{
			get
			{
				ParticleBoxShapeOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleBoxShapeOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleBoxShapeOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitterBoxShape managedInstance, ref ParticleBoxShapeOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleEmitterBoxShape managedInstance);
	}

	/** @} */
}

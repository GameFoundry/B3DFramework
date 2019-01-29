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

	/// <summary>Handles spawning of new particles using the specified parameters and shape.</summary>
	[ShowInInspector]
	public partial class ParticleEmitter : ScriptObject
	{
		private ParticleEmitter(bool __dummy0) { }

		/// <summary>Creates a new emitter.</summary>
		public ParticleEmitter()
		{
			Internal_create(this);
		}

		/// <summary>Shape over which to emit the particles.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleEmitterShape Shape
		{
			get { return Internal_getShape(mCachedPtr); }
			set { Internal_setShape(mCachedPtr, value); }
		}

		/// <summary>Determines the number of particles that are emitted every second.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution EmissionRate
		{
			get { return Internal_getEmissionRate(mCachedPtr); }
			set { Internal_setEmissionRate(mCachedPtr, value); }
		}

		/// <summary>Determines discrete intervals to emit particles.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleBurst[] EmissionBursts
		{
			get { return Internal_getEmissionBursts(mCachedPtr); }
			set { Internal_setEmissionBursts(mCachedPtr, value); }
		}

		/// <summary>Determines the lifetime of particles when they are initially spawned, in seconds.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution InitialLifetime
		{
			get { return Internal_getInitialLifetime(mCachedPtr); }
			set { Internal_setInitialLifetime(mCachedPtr, value); }
		}

		/// <summary>
		/// Sets the initial speed of the particles, in meters/second. The speed is applied along the particle's velocity 
		/// direction, which is determined by the emission shape and potentially other properties.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution InitialSpeed
		{
			get { return Internal_getInitialSpeed(mCachedPtr); }
			set { Internal_setInitialSpeed(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the size of the particles when initially spawned. The size is applied uniformly in all dimensions. Only 
		/// used if 3D size is disabled.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution InitialSize
		{
			get { return Internal_getInitialSize(mCachedPtr); }
			set { Internal_setInitialSize(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the size of the particles when initially spawned. Size can be specified for each dimension separately. 
		/// Only used if 3D size is enabled.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public Vector3Distribution InitialSize3D
		{
			get { return Internal_getInitialSize3D(mCachedPtr); }
			set { Internal_setInitialSize3D(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the initial particle size be applied uniformly (if disabled), or evaluated separately for each 
		/// dimension (if enabled).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Use3DSize
		{
			get { return Internal_getUse3DSize(mCachedPtr); }
			set { Internal_setUse3DSize(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the rotation of the particles when initially spawned, in degrees. The rotation is applied around the  
		/// particle's local Z axis. Only used if 3D rotation is disabled.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution InitialRotation
		{
			get { return Internal_getInitialRotation(mCachedPtr); }
			set { Internal_setInitialRotation(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the rotation of the particles when initially spawned, in Euler angles. Only used if 3D rotation is enabled.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public Vector3Distribution InitialRotation3D
		{
			get { return Internal_getInitialRotation3D(mCachedPtr); }
			set { Internal_setInitialRotation3D(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the initial particle rotation be a single angle applied around a Z axis (if disabled), or a  set of 
		/// Euler angles that allow you to rotate around every axis (if enabled).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Use3DRotation
		{
			get { return Internal_getUse3DRotation(mCachedPtr); }
			set { Internal_setUse3DRotation(mCachedPtr, value); }
		}

		/// <summary>Determines the initial color (in RGB channels) and transparency (in A channel) of particles.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ColorDistribution InitialColor
		{
			get { return Internal_getInitialColor(mCachedPtr); }
			set { Internal_setInitialColor(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a range of values determining a random offset to apply to particle position after it has been emitted.  
		/// Offset will be randomly selected in all three axes in range [-value, value].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float RandomOffset
		{
			get { return Internal_getRandomOffset(mCachedPtr); }
			set { Internal_setRandomOffset(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should particle U texture coordinate be randomly flipped, mirroring the image. The value represents a 
		/// percent of particles that should be flipped, in range [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FlipU
		{
			get { return Internal_getFlipU(mCachedPtr); }
			set { Internal_setFlipU(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should particle V texture coordinate be randomly flipped, mirroring the image. The value represents a 
		/// percent of particles that should be flipped, in range [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FlipV
		{
			get { return Internal_getFlipV(mCachedPtr); }
			set { Internal_setFlipV(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setShape(IntPtr thisPtr, ParticleEmitterShape shape);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleEmitterShape Internal_getShape(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setEmissionRate(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_getEmissionRate(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setEmissionBursts(IntPtr thisPtr, ParticleBurst[] bursts);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleBurst[] Internal_getEmissionBursts(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInitialLifetime(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_getInitialLifetime(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInitialSpeed(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_getInitialSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInitialSize(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_getInitialSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInitialSize3D(IntPtr thisPtr, Vector3Distribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Distribution Internal_getInitialSize3D(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setUse3DSize(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getUse3DSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInitialRotation(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_getInitialRotation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInitialRotation3D(IntPtr thisPtr, Vector3Distribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Distribution Internal_getInitialRotation3D(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setUse3DRotation(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getUse3DRotation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInitialColor(IntPtr thisPtr, ColorDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorDistribution Internal_getInitialColor(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRandomOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getRandomOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFlipU(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getFlipU(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFlipV(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getFlipV(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleEmitter managedInstance);
	}

	/** @} */
}

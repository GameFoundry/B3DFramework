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

	/// <summary>Handles spawning of new particles using the specified parameters and shape.</summary>
	[ShowInInspector]
	public partial class ParticleEmitter : ScriptObject
	{
		private ParticleEmitter(bool __dummy0) { }

		/// <summary>Creates a new emitter.</summary>
		public ParticleEmitter()
		{
			Internal_Create(this);
		}

		/// <summary>Shape over which to emit the particles.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleEmitterShape Shape
		{
			get { return Internal_GetShape(mCachedPtr); }
			set { Internal_SetShape(mCachedPtr, value); }
		}

		/// <summary>Determines the number of particles that are emitted every second.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution EmissionRate
		{
			get { return Internal_GetEmissionRate(mCachedPtr); }
			set { Internal_SetEmissionRate(mCachedPtr, value); }
		}

		/// <summary>Determines discrete intervals to emit particles.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleBurst[] EmissionBursts
		{
			get { return Internal_GetEmissionBursts(mCachedPtr); }
			set { Internal_SetEmissionBursts(mCachedPtr, value); }
		}

		/// <summary>Determines the lifetime of particles when they are initially spawned, in seconds.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution InitialLifetime
		{
			get { return Internal_GetInitialLifetime(mCachedPtr); }
			set { Internal_SetInitialLifetime(mCachedPtr, value); }
		}

		/// <summary>
		/// Sets the initial speed of the particles, in meters/second. The speed is applied along the particle&apos;s velocity 
		/// direction, which is determined by the emission shape and potentially other properties.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution InitialSpeed
		{
			get { return Internal_GetInitialSpeed(mCachedPtr); }
			set { Internal_SetInitialSpeed(mCachedPtr, value); }
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
			get { return Internal_GetInitialSize(mCachedPtr); }
			set { Internal_SetInitialSize(mCachedPtr, value); }
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
			get { return Internal_GetInitialSize3D(mCachedPtr); }
			set { Internal_SetInitialSize3D(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the initial particle size be applied uniformly (if disabled), or evaluated separately for each 
		/// dimension (if enabled).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Use3DSize
		{
			get { return Internal_GetUse3DSize(mCachedPtr); }
			set { Internal_SetUse3DSize(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the rotation of the particles when initially spawned, in degrees. The rotation is applied around the 
		/// particle&apos;s local Z axis. Only used if 3D rotation is disabled.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FloatDistribution InitialRotation
		{
			get { return Internal_GetInitialRotation(mCachedPtr); }
			set { Internal_SetInitialRotation(mCachedPtr, value); }
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
			get { return Internal_GetInitialRotation3D(mCachedPtr); }
			set { Internal_SetInitialRotation3D(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the initial particle rotation be a single angle applied around a Z axis (if disabled), or a set of 
		/// Euler angles that allow you to rotate around every axis (if enabled).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Use3DRotation
		{
			get { return Internal_GetUse3DRotation(mCachedPtr); }
			set { Internal_SetUse3DRotation(mCachedPtr, value); }
		}

		/// <summary>Determines the initial color (in RGB channels) and transparency (in A channel) of particles.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ColorDistribution InitialColor
		{
			get { return Internal_GetInitialColor(mCachedPtr); }
			set { Internal_SetInitialColor(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a range of values determining a random offset to apply to particle position after it has been emitted. 
		/// Offset will be randomly selected in all three axes in range [-value, value].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float RandomOffset
		{
			get { return Internal_GetRandomOffset(mCachedPtr); }
			set { Internal_SetRandomOffset(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should particle U texture coordinate be randomly flipped, mirroring the image. The value represents a 
		/// percent of particles that should be flipped, in range [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FlipU
		{
			get { return Internal_GetFlipU(mCachedPtr); }
			set { Internal_SetFlipU(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should particle V texture coordinate be randomly flipped, mirroring the image. The value represents a 
		/// percent of particles that should be flipped, in range [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FlipV
		{
			get { return Internal_GetFlipV(mCachedPtr); }
			set { Internal_SetFlipV(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetShape(IntPtr thisPtr, ParticleEmitterShape shape);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleEmitterShape Internal_GetShape(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEmissionRate(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_GetEmissionRate(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEmissionBursts(IntPtr thisPtr, ParticleBurst[] bursts);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleBurst[] Internal_GetEmissionBursts(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetInitialLifetime(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_GetInitialLifetime(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetInitialSpeed(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_GetInitialSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetInitialSize(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_GetInitialSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetInitialSize3D(IntPtr thisPtr, Vector3Distribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Distribution Internal_GetInitialSize3D(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUse3DSize(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetUse3DSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetInitialRotation(IntPtr thisPtr, FloatDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FloatDistribution Internal_GetInitialRotation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetInitialRotation3D(IntPtr thisPtr, Vector3Distribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Distribution Internal_GetInitialRotation3D(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUse3DRotation(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetUse3DRotation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetInitialColor(IntPtr thisPtr, ColorDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorDistribution Internal_GetInitialColor(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRandomOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRandomOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlipU(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFlipU(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlipV(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFlipV(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleEmitter managedInstance);
	}

	/** @} */
}

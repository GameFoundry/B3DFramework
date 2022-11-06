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

	/// <summary>Settings used for controlling particle system GPU simulation.</summary>
	[ShowInInspector]
	public partial class ParticleGpuSimulationSettings : ScriptObject
	{
		private ParticleGpuSimulationSettings(bool __dummy0) { }
		protected ParticleGpuSimulationSettings() { }

		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ParticleVectorFieldSettings VectorField
		{
			get { return Internal_GetVectorField(mCachedPtr); }
			set { Internal_SetVectorField(mCachedPtr, value); }
		}

		/// <summary>Determines particle color, evaluated over the particle lifetime.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ColorDistribution ColorOverLifetime
		{
			get { return Internal_GetColorOverLifetime(mCachedPtr); }
			set { Internal_SetColorOverLifetime(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines particle size, evaluated over the particle lifetime. Multiplied by the initial particle size.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public Vector2Distribution SizeScaleOverLifetime
		{
			get { return Internal_GetSizeScaleOverLifetime(mCachedPtr); }
			set { Internal_SetSizeScaleOverLifetime(mCachedPtr, value); }
		}

		/// <summary>Constant acceleration to apply for each step of the simulation.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Acceleration
		{
			get
			{
				Vector3 temp;
				Internal_GetAcceleration(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetAcceleration(mCachedPtr, ref value); }
		}

		/// <summary>Amount of resistance to apply in the direction opposite of the particle&apos;s velocity.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Drag
		{
			get { return Internal_GetDrag(mCachedPtr); }
			set { Internal_SetDrag(mCachedPtr, value); }
		}

		/// <summary>Settings controlling particle depth buffer collisions.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ParticleDepthCollisionSettings DepthCollision
		{
			get { return Internal_GetDepthCollision(mCachedPtr); }
			set { Internal_SetDepthCollision(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleVectorFieldSettings Internal_GetVectorField(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVectorField(IntPtr thisPtr, ParticleVectorFieldSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorDistribution Internal_GetColorOverLifetime(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetColorOverLifetime(IntPtr thisPtr, ColorDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2Distribution Internal_GetSizeScaleOverLifetime(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSizeScaleOverLifetime(IntPtr thisPtr, Vector2Distribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetAcceleration(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAcceleration(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetDrag(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDrag(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleDepthCollisionSettings Internal_GetDepthCollision(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDepthCollision(IntPtr thisPtr, ParticleDepthCollisionSettings value);
	}

	/** @} */
}

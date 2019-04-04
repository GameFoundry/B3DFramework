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
			get { return Internal_getvectorField(mCachedPtr); }
			set { Internal_setvectorField(mCachedPtr, value); }
		}

		/// <summary>Determines particle color, evaluated over the particle lifetime.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ColorDistribution ColorOverLifetime
		{
			get { return Internal_getcolorOverLifetime(mCachedPtr); }
			set { Internal_setcolorOverLifetime(mCachedPtr, value); }
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
			get { return Internal_getsizeScaleOverLifetime(mCachedPtr); }
			set { Internal_setsizeScaleOverLifetime(mCachedPtr, value); }
		}

		/// <summary>Constant acceleration to apply for each step of the simulation.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Acceleration
		{
			get
			{
				Vector3 temp;
				Internal_getacceleration(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setacceleration(mCachedPtr, ref value); }
		}

		/// <summary>Amount of resistance to apply in the direction opposite of the particle&apos;s velocity.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Drag
		{
			get { return Internal_getdrag(mCachedPtr); }
			set { Internal_setdrag(mCachedPtr, value); }
		}

		/// <summary>Settings controlling particle depth buffer collisions.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ParticleDepthCollisionSettings DepthCollision
		{
			get { return Internal_getdepthCollision(mCachedPtr); }
			set { Internal_setdepthCollision(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleVectorFieldSettings Internal_getvectorField(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setvectorField(IntPtr thisPtr, ParticleVectorFieldSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorDistribution Internal_getcolorOverLifetime(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcolorOverLifetime(IntPtr thisPtr, ColorDistribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2Distribution Internal_getsizeScaleOverLifetime(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setsizeScaleOverLifetime(IntPtr thisPtr, Vector2Distribution value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getacceleration(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setacceleration(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getdrag(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setdrag(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleDepthCollisionSettings Internal_getdepthCollision(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setdepthCollision(IntPtr thisPtr, ParticleDepthCollisionSettings value);
	}

	/** @} */
}

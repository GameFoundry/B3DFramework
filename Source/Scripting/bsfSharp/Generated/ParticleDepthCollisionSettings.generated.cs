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

	/// <summary>Controls depth buffer collisions for GPU simulated particles.</summary>
	[ShowInInspector]
	public partial class ParticleDepthCollisionSettings : ScriptObject
	{
		private ParticleDepthCollisionSettings(bool __dummy0) { }

		public ParticleDepthCollisionSettings()
		{
			Internal_ParticleDepthCollisionSettings(this);
		}

		/// <summary>Determines if depth collisions are enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_GetEnabled(mCachedPtr); }
			set { Internal_SetEnabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the elasticity (bounciness) of the particle collision. Lower values make the collision less bouncy and 
		/// higher values more.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Restitution
		{
			get { return Internal_GetRestitution(mCachedPtr); }
			set { Internal_SetRestitution(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how much velocity should a particle lose after a collision, in percent of its current velocity. In range 
		/// [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Dampening
		{
			get { return Internal_GetDampening(mCachedPtr); }
			set { Internal_SetDampening(mCachedPtr, value); }
		}

		/// <summary>Scale which to apply to particle size in order to determine the collision radius.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float RadiusScale
		{
			get { return Internal_GetRadiusScale(mCachedPtr); }
			set { Internal_SetRadiusScale(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ParticleDepthCollisionSettings(ParticleDepthCollisionSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRestitution(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRestitution(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetDampening(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDampening(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRadiusScale(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRadiusScale(IntPtr thisPtr, float value);
	}

	/** @} */
}

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
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the elasticity (bounciness) of the particle collision. Lower values make the collision less bouncy and 
		/// higher values more.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Restitution
		{
			get { return Internal_getrestitution(mCachedPtr); }
			set { Internal_setrestitution(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how much velocity should a particle lose after a collision, in percent of its current velocity. In range 
		/// [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Dampening
		{
			get { return Internal_getdampening(mCachedPtr); }
			set { Internal_setdampening(mCachedPtr, value); }
		}

		/// <summary>Scale which to apply to particle size in order to determine the collision radius.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float RadiusScale
		{
			get { return Internal_getradiusScale(mCachedPtr); }
			set { Internal_setradiusScale(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ParticleDepthCollisionSettings(ParticleDepthCollisionSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getrestitution(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setrestitution(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getdampening(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setdampening(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getradiusScale(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setradiusScale(IntPtr thisPtr, float value);
	}

	/** @} */
}

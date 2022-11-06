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

	[ShowInInspector]
	public partial class ParticleSystem : Component
	{
		private ParticleSystem(bool __dummy0) { }
		protected ParticleSystem() { }

		[ShowInInspector]
		[Inline]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ParticleSystemSettings Settings
		{
			get { return Internal_GetSettings(mCachedPtr); }
			set { Internal_SetSettings(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ParticleGpuSimulationSettings GpuSimulationSettings
		{
			get { return Internal_GetGpuSimulationSettings(mCachedPtr); }
			set { Internal_SetGpuSimulationSettings(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ParticleEmitter[] Emitters
		{
			get { return Internal_GetEmitters(mCachedPtr); }
			set { Internal_SetEmitters(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ParticleEvolver[] Evolvers
		{
			get { return Internal_GetEvolvers(mCachedPtr); }
			set { Internal_SetEvolvers(mCachedPtr, value); }
		}

		[ShowInInspector]
		[LayerMask]
		[NativeWrapper]
		public ulong Layer
		{
			get { return Internal_GetLayer(mCachedPtr); }
			set { Internal_SetLayer(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disabled preview mode. Preview mode allows the particle system to play while the game is not running, 
		/// primarily for preview purposes in the editor. Returns true if the preview mode was enabled, false if it was disabled 
		/// or enabling preview failed.
		/// </summary>
		internal bool TogglePreviewMode(bool enabled)
		{
			return Internal_TogglePreviewModeInternal(mCachedPtr, enabled);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSettings(IntPtr thisPtr, ParticleSystemSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleSystemSettings Internal_GetSettings(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGpuSimulationSettings(IntPtr thisPtr, ParticleGpuSimulationSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleGpuSimulationSettings Internal_GetGpuSimulationSettings(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEmitters(IntPtr thisPtr, ParticleEmitter[] emitters);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleEmitter[] Internal_GetEmitters(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEvolvers(IntPtr thisPtr, ParticleEvolver[] evolvers);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleEvolver[] Internal_GetEvolvers(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLayer(IntPtr thisPtr, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_TogglePreviewModeInternal(IntPtr thisPtr, bool enabled);
	}

	/** @} */
}

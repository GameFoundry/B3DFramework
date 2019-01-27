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
	/// Controls spawning, evolution and rendering of particles. Particles can be 2D or 3D, with a variety of rendering 
	/// options. Particle system should be used for rendering objects that cannot properly be represented using static or 
	/// animated meshes, like liquids, smoke or flames.
	///
	/// The particle system requires you to specify at least one ParticleEmitter, which controls how are new particles 
	/// generated. You will also want to specify one or more ParticleEvolver%s, which change particle properties over time.
	/// </summary>
	[ShowInInspector]
	public partial class ParticleSystem : Component
	{
		private ParticleSystem(bool __dummy0) { }
		protected ParticleSystem() { }

		/// <summary>Determines general purpose settings that apply to the particle system.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ParticleSystemSettings Settings
		{
			get { return Internal_getSettings(mCachedPtr); }
			set { Internal_setSettings(mCachedPtr, value); }
		}

		/// <summary>Determines settings that control particle GPU simulation.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ParticleGpuSimulationSettings GpuSimulationSettings
		{
			get { return Internal_getGpuSimulationSettings(mCachedPtr); }
			set { Internal_setGpuSimulationSettings(mCachedPtr, value); }
		}

		/// <summary>
		/// Set of objects that determine initial position, normal and other properties of newly spawned particles. Each particle 
		/// system must have at least one emitter.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleEmitter[] Emitters
		{
			get { return Internal_getEmitters(mCachedPtr); }
			set { Internal_setEmitters(mCachedPtr, value); }
		}

		/// <summary>
		/// Set of objects that determine how particle properties change during their lifetime. Evolvers only affect CPU 
		/// simulated particles.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleEvolver[] Evolvers
		{
			get { return Internal_getEvolvers(mCachedPtr); }
			set { Internal_setEvolvers(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the layer bitfield that controls whether a system is considered visible in a specific camera.  Layer must 
		/// match camera layer in order for the camera to render the component.
		/// </summary>
		[ShowInInspector]
		[LayerMask]
		[NativeWrapper]
		public ulong Layer
		{
			get { return Internal_getLayer(mCachedPtr); }
			set { Internal_setLayer(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disabled preview mode. Preview mode allows the particle system to play while the game is not running,  
		/// primarily for preview purposes in the editor. Returns true if the preview mode was enabled, false if it was disabled 
		/// or enabling preview failed.
		/// </summary>
		internal bool TogglePreviewMode(bool enabled)
		{
			return Internal__togglePreviewMode(mCachedPtr, enabled);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSettings(IntPtr thisPtr, ParticleSystemSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleSystemSettings Internal_getSettings(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setGpuSimulationSettings(IntPtr thisPtr, ParticleGpuSimulationSettings settings);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleGpuSimulationSettings Internal_getGpuSimulationSettings(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setEmitters(IntPtr thisPtr, ParticleEmitter[] emitters);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleEmitter[] Internal_getEmitters(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setEvolvers(IntPtr thisPtr, ParticleEvolver[] evolvers);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleEvolver[] Internal_getEvolvers(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLayer(IntPtr thisPtr, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_getLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal__togglePreviewMode(IntPtr thisPtr, bool enabled);
	}

	/** @} */
}

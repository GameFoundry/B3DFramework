//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Generic settings used for controlling a ParticleSystem.</summary>
	[ShowInInspector]
	public partial class ParticleSystemSettings : ScriptObject
	{
		private ParticleSystemSettings(bool __dummy0) { }
		protected ParticleSystemSettings() { }

		/// <summary>Material to render the particles with.</summary>
		[ShowInInspector]
		[LoadOnAssign]
		[NativeWrapper]
		public RRef<Material> Material
		{
			get { return Internal_GetMaterial(mCachedPtr); }
			set { Internal_SetMaterial(mCachedPtr, value); }
		}

		/// <summary>Mesh used for representing individual particles when using the Mesh rendering mode.</summary>
		[ShowInInspector]
		[Order(2)]
		[LoadOnAssign]
		[NativeWrapper]
		public RRef<Mesh> Mesh
		{
			get { return Internal_GetMesh(mCachedPtr); }
			set { Internal_SetMesh(mCachedPtr, value); }
		}

		/// <summary>Determines in which space are particles in.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleSimulationSpace SimulationSpace
		{
			get { return Internal_GetSimulationSpace(mCachedPtr); }
			set { Internal_SetSimulationSpace(mCachedPtr, value); }
		}

		/// <summary>Determines how are particles oriented when rendering.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleOrientation Orientation
		{
			get { return Internal_GetOrientation(mCachedPtr); }
			set { Internal_SetOrientation(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the time period during which the system runs, in seconds. This effects evaluation of distributions with 
		/// curves using particle system time for evaluation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Duration
		{
			get { return Internal_GetDuration(mCachedPtr); }
			set { Internal_SetDuration(mCachedPtr, value); }
		}

		/// <summary>Determines should the particle system time wrap around once it reaches its duration.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool IsLooping
		{
			get { return Internal_GetIsLooping(mCachedPtr); }
			set { Internal_SetIsLooping(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum number of particles that can ever be active in this system. This number is ignored if GPU 
		/// simulation is enabled, and instead particle count is instead only limited by the size of the internal buffers (shared 
		/// between all particle systems).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int MaxParticles
		{
			get { return Internal_GetMaxParticles(mCachedPtr); }
			set { Internal_SetMaxParticles(mCachedPtr, value); }
		}

		/// <summary>
		/// If true the particle system will be simulated on the GPU. This allows much higher particle counts at lower 
		/// performance cost. GPU simulation ignores any provided evolvers and instead uses ParticleGpuSimulationSettings to 
		/// customize the GPU simulation.
		/// </summary>
		[ShowInInspector]
		[Order(1)]
		[Category("Advanced")]
		[NativeWrapper]
		public bool GpuSimulation
		{
			get { return Internal_GetGpuSimulation(mCachedPtr); }
			set { Internal_SetGpuSimulation(mCachedPtr, value); }
		}

		/// <summary>Determines how is each particle represented on the screen.</summary>
		[ShowInInspector]
		[Order(2)]
		[NativeWrapper]
		public ParticleRenderMode RenderMode
		{
			get { return Internal_GetRenderMode(mCachedPtr); }
			set { Internal_SetRenderMode(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the particles only be allowed to orient themselves around the Y axis, or freely. Ignored if using 
		/// the Plane orientation mode.
		/// </summary>
		[ShowInInspector]
		[Order(2)]
		[NativeWrapper]
		public bool OrientationLockY
		{
			get { return Internal_GetOrientationLockY(mCachedPtr); }
			set { Internal_SetOrientationLockY(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a normal of the plane to orient particles towards. Only used if particle orientation mode is set to 
		/// ParticleOrientation::Plane.
		/// </summary>
		[ShowInInspector]
		[Order(2)]
		[NativeWrapper]
		public Vector3 OrientationPlaneNormal
		{
			get
			{
				Vector3 temp;
				Internal_GetOrientationPlaneNormal(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOrientationPlaneNormal(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines how (and if) are particles sorted. Sorting controls in what order are particles rendered. If GPU 
		/// simulation is enabled only distance based sorting is supported.
		/// </summary>
		[ShowInInspector]
		[Order(2)]
		[NativeWrapper]
		public ParticleSortMode SortMode
		{
			get { return Internal_GetSortMode(mCachedPtr); }
			set { Internal_SetSortMode(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should an automatic seed be used for the internal random number generator. This ensures the particle 
		/// system yields different results each time it is ran.
		/// </summary>
		[ShowInInspector]
		[Order(2)]
		[NativeWrapper]
		public bool UseAutomaticSeed
		{
			get { return Internal_GetUseAutomaticSeed(mCachedPtr); }
			set { Internal_SetUseAutomaticSeed(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the seed to use for the internal random number generator. Allows you to guarantee identical behaviour 
		/// between different runs. Only relevant if automatic seed is disabled.
		/// </summary>
		[ShowInInspector]
		[Order(2)]
		[NativeWrapper]
		public int ManualSeed
		{
			get { return Internal_GetManualSeed(mCachedPtr); }
			set { Internal_SetManualSeed(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the particle system bounds be automatically calculated, or should the fixed value provided be used. 
		/// Bounds are used primarily for culling purposes. Note that automatic bounds are not supported when GPU simulation is 
		/// enabled.
		/// </summary>
		[ShowInInspector]
		[Order(2)]
		[NativeWrapper]
		public bool UseAutomaticBounds
		{
			get { return Internal_GetUseAutomaticBounds(mCachedPtr); }
			set { Internal_SetUseAutomaticBounds(mCachedPtr, value); }
		}

		/// <summary>
		/// Custom bounds to use them <see cref="UseAutomaticBounds"/> is disabled. The bounds are in the simulation space of the 
		/// particle system.
		/// </summary>
		[ShowInInspector]
		[Order(2)]
		[NativeWrapper]
		public AABox CustomBounds
		{
			get
			{
				AABox temp;
				Internal_GetCustomBounds(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetCustomBounds(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_GetMaterial(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaterial(IntPtr thisPtr, RRef<Material> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Mesh> Internal_GetMesh(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMesh(IntPtr thisPtr, RRef<Mesh> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleSimulationSpace Internal_GetSimulationSpace(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSimulationSpace(IntPtr thisPtr, ParticleSimulationSpace value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleOrientation Internal_GetOrientation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOrientation(IntPtr thisPtr, ParticleOrientation value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetDuration(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDuration(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetIsLooping(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIsLooping(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetMaxParticles(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxParticles(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetGpuSimulation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGpuSimulation(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleRenderMode Internal_GetRenderMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRenderMode(IntPtr thisPtr, ParticleRenderMode value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetOrientationLockY(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOrientationLockY(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOrientationPlaneNormal(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOrientationPlaneNormal(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleSortMode Internal_GetSortMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSortMode(IntPtr thisPtr, ParticleSortMode value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetUseAutomaticSeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUseAutomaticSeed(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetManualSeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetManualSeed(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetUseAutomaticBounds(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUseAutomaticBounds(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCustomBounds(IntPtr thisPtr, out AABox __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCustomBounds(IntPtr thisPtr, ref AABox value);
	}

	/** @} */
}

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

	/// <summary>Generic settings used for controlling a ParticleSystem.</summary>
	[ShowInInspector]
	public partial class ParticleSystemSettings : ScriptObject
	{
		private ParticleSystemSettings(bool __dummy0) { }
		protected ParticleSystemSettings() { }

		/// <summary>Material to render the particles with.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Material> Material
		{
			get { return Internal_getmaterial(mCachedPtr); }
			set { Internal_setmaterial(mCachedPtr, value); }
		}

		/// <summary>Mesh used for representing individual particles when using the Mesh rendering mode.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Mesh> Mesh
		{
			get { return Internal_getmesh(mCachedPtr); }
			set { Internal_setmesh(mCachedPtr, value); }
		}

		/// <summary>
		/// If true the particle system will be simulated on the GPU. This allows much higher particle counts at lower 
		/// performance cost. GPU simulation ignores any provided evolvers and instead uses ParticleGpuSimulationSettings to 
		/// customize the GPU simulation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool GpuSimulation
		{
			get { return Internal_getgpuSimulation(mCachedPtr); }
			set { Internal_setgpuSimulation(mCachedPtr, value); }
		}

		/// <summary>Determines in which space are particles in.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleSimulationSpace SimulationSpace
		{
			get { return Internal_getsimulationSpace(mCachedPtr); }
			set { Internal_setsimulationSpace(mCachedPtr, value); }
		}

		/// <summary>Determines how are particles oriented when rendering.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleOrientation Orientation
		{
			get { return Internal_getorientation(mCachedPtr); }
			set { Internal_setorientation(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the particles only be allowed to orient themselves around the Y axis, or freely. Ignored if using 
		/// the Plane orientation mode.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool OrientationLockY
		{
			get { return Internal_getorientationLockY(mCachedPtr); }
			set { Internal_setorientationLockY(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a normal of the plane to orient particles towards. Only used if particle orientation mode is set to  
		/// ParticleOrientation::Plane.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 OrientationPlaneNormal
		{
			get
			{
				Vector3 temp;
				Internal_getorientationPlaneNormal(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setorientationPlaneNormal(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines how (and if) are particles sorted. Sorting controls in what order are particles rendered.  If GPU 
		/// simulation is enabled only distance based sorting is supported.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleSortMode SortMode
		{
			get { return Internal_getsortMode(mCachedPtr); }
			set { Internal_setsortMode(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the time period during which the system runs, in seconds. This effects evaluation of distributions with 
		/// curves using particle system time for evaluation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Duration
		{
			get { return Internal_getduration(mCachedPtr); }
			set { Internal_setduration(mCachedPtr, value); }
		}

		/// <summary>Determines should the particle system time wrap around once it reaches its duration.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool IsLooping
		{
			get { return Internal_getisLooping(mCachedPtr); }
			set { Internal_setisLooping(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum number of particles that can ever be active in this system. This number is ignored if GPU 
		/// simulation is enabled, and instead particle count is instead only limited by the size of the internal buffers (shared 
		/// between all particle systems).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint MaxParticles
		{
			get { return Internal_getmaxParticles(mCachedPtr); }
			set { Internal_setmaxParticles(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should an automatic seed be used for the internal random number generator. This ensures the particle 
		/// system yields different results each time it is ran.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool UseAutomaticSeed
		{
			get { return Internal_getuseAutomaticSeed(mCachedPtr); }
			set { Internal_setuseAutomaticSeed(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the seed to use for the internal random number generator. Allows you to guarantee identical behaviour 
		/// between different runs. Only relevant if automatic seed is disabled.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint ManualSeed
		{
			get { return Internal_getmanualSeed(mCachedPtr); }
			set { Internal_setmanualSeed(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the particle system bounds be automatically calculated, or should the fixed value provided be used. 
		/// Bounds are used primarily for culling purposes. Note that automatic bounds are not supported when GPU simulation is 
		/// enabled.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool UseAutomaticBounds
		{
			get { return Internal_getuseAutomaticBounds(mCachedPtr); }
			set { Internal_setuseAutomaticBounds(mCachedPtr, value); }
		}

		/// <summary>
		/// Custom bounds to use them <see cref="useAutomaticBounds"/> is disabled. The bounds are in the simulation space of the 
		/// particle system.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public AABox CustomBounds
		{
			get
			{
				AABox temp;
				Internal_getcustomBounds(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setcustomBounds(mCachedPtr, ref value); }
		}

		/// <summary>Determines how is each particle represented on the screen.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleRenderMode RenderMode
		{
			get { return Internal_getrenderMode(mCachedPtr); }
			set { Internal_setrenderMode(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_getmaterial(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaterial(IntPtr thisPtr, RRef<Material> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Mesh> Internal_getmesh(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmesh(IntPtr thisPtr, RRef<Mesh> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getgpuSimulation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setgpuSimulation(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleSimulationSpace Internal_getsimulationSpace(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setsimulationSpace(IntPtr thisPtr, ParticleSimulationSpace value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleOrientation Internal_getorientation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setorientation(IntPtr thisPtr, ParticleOrientation value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getorientationLockY(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setorientationLockY(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getorientationPlaneNormal(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setorientationPlaneNormal(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleSortMode Internal_getsortMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setsortMode(IntPtr thisPtr, ParticleSortMode value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getduration(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setduration(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getisLooping(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setisLooping(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getmaxParticles(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaxParticles(IntPtr thisPtr, uint value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getuseAutomaticSeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setuseAutomaticSeed(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getmanualSeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmanualSeed(IntPtr thisPtr, uint value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getuseAutomaticBounds(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setuseAutomaticBounds(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getcustomBounds(IntPtr thisPtr, out AABox __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcustomBounds(IntPtr thisPtr, ref AABox value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ParticleRenderMode Internal_getrenderMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setrenderMode(IntPtr thisPtr, ParticleRenderMode value);
	}

	/** @} */
}

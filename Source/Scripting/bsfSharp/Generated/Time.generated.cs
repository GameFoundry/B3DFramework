//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/// <summary>Manages all time related functionality.</summary>
	[ShowInInspector]
	public partial class Time : ScriptObject
	{
		private Time(bool __dummy0) { }
		protected Time() { }

		/// <summary>Gets the time elapsed since application start. Only gets updated once per frame.</summary>
		/// <returns>
		/// The time since application start, in seconds. This is real time, unaffected by simulation time scale.
		/// </returns>
		[NativeWrapper]
		public static float RealTimeInSeconds
		{
			get { return Internal_GetRealTimeInSeconds(); }
		}

		/// <summary>Gets the time elapsed since application start. Only gets updated once per frame.</summary>
		/// <returns>
		/// The time since application start, in miliseconds. This is real time, unaffected by simulation time scale.
		/// </returns>
		[NativeWrapper]
		public static ulong RealTimeInMilliseconds
		{
			get { return Internal_GetRealTimeInMilliseconds(); }
		}

		/// <summary>
		/// Gets the time since the simulation started playing, multiplied by the time scale factor. In editor this will reset to 
		/// zero every time you start playing in editor, and in a standalone application this will be similar to 
		/// GetRealTimeInSeconds(), except simulation time can be sped up/down, or stopped entirely by setting the time scale.
		/// </summary>
		/// <returns>Time since game start, affected by simulation time scale.</returns>
		[NativeWrapper]
		public static float SimulationTimeInSeconds
		{
			get { return Internal_GetSimulationTimeInSeconds(); }
		}

		/// <summary>
		/// Allows you to speed time up or down, or completely pause it by providing zero. Must be zero or larger.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public static float SimulationTimeScale
		{
			get { return Internal_GetSimulationTimeScale(); }
			set { Internal_SetSimulationTimeScale(value); }
		}

		/// <summary>Gets the time since last frame was executed. Only gets updated once per frame.</summary>
		/// <returns>Time since last frame was executed, in seconds.</returns>
		[NativeWrapper]
		public static float FrameDelta
		{
			get { return Internal_GetFrameDelta(); }
		}

		/// <summary>Returns the sequential index of the current frame. First frame is 0.</summary>
		/// <returns>The current frame.</returns>
		[NativeWrapper]
		public static ulong CurrentFrameIndex
		{
			get { return Internal_GetCurrentFrameIndex(); }
		}

		/// <summary>
		/// Returns the precise time since application start, in microseconds. Unlike other time methods this is not only updated 
		/// every frame, but will return exact time at the moment it is called.
		/// </summary>
		/// <returns>Time in microseconds.</returns>
		[NativeWrapper]
		public static ulong TimePrecise
		{
			get { return Internal_GetTimePrecise(); }
		}

		/// <summary>
		/// Resets the simulation time to zero. Primarily used for editor purposes for resetting the time when ending play in 
		/// editor.
		/// </summary>
		public static void ResetSimulationTime()
		{
			Internal_ResetSimulationTime();
		}

		/// <summary>Pauses or unpauses the simulation time. This is equivalent to setting the time scale to 0.</summary>
		public static void SetSimulationTimePaused(bool paused)
		{
			Internal_SetSimulationTimePaused(paused);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRealTimeInSeconds();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetRealTimeInMilliseconds();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSimulationTimeInSeconds();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSimulationTimeScale(float scale);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSimulationTimeScale();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ResetSimulationTime();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSimulationTimePaused(bool paused);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFrameDelta();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetCurrentFrameIndex();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetTimePrecise();
	}

	/** @} */
}

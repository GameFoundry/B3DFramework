//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/// <summary>Manages global time related functionality.</summary>
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

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRealTimeInSeconds();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetRealTimeInMilliseconds();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFrameDelta();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetCurrentFrameIndex();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetTimePrecise();
	}

	/** @} */
}

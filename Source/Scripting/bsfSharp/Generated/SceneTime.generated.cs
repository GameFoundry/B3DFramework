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

	/// <summary>Manages simulation time for a particular scene. This time runs only while scene is simulating.</summary>
	[ShowInInspector]
	public partial class SceneTime : ScriptObject
	{
		private SceneTime(bool __dummy0) { }
		protected SceneTime() { }

		/// <summary>
		/// Gets the time since the simulation started playing, multiplied by the time scale factor. In editor this will reset to 
		/// zero every time you start playing in editor, and in a standalone application this will be similar to 
		/// GetRealTimeInSeconds(), except simulation time can be sped up/down, or stopped entirely by setting the time scale.
		/// </summary>
		/// <returns>Time since game start, affected by simulation time scale.</returns>
		[NativeWrapper]
		public float TimeInSeconds
		{
			get { return Internal_GetTimeInSeconds(mCachedPtr); }
		}

		/// <summary>
		/// Allows you to speed time up or down, or completely pause it by providing zero. Must be zero or larger.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Scale
		{
			get { return Internal_GetScale(mCachedPtr); }
			set { Internal_SetScale(mCachedPtr, value); }
		}

		/// <summary>Gets the time since last frame was executed. Only gets updated once per frame.</summary>
		/// <returns>Time since last frame was executed, in seconds.</returns>
		[NativeWrapper]
		public float FrameDelta
		{
			get { return Internal_GetFrameDelta(mCachedPtr); }
		}

		/// <summary>
		/// Resets the simulation time to zero. Primarily used for editor purposes for resetting the time when ending play in 
		/// editor.
		/// </summary>
		public void Reset()
		{
			Internal_Reset(mCachedPtr);
		}

		/// <summary>Pauses or unpauses the simulation time. This is equivalent to setting the time scale to 0.</summary>
		public void SetPaused(bool paused)
		{
			Internal_SetPaused(mCachedPtr, paused);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetTimeInSeconds(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetScale(IntPtr thisPtr, float scale);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetScale(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Reset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPaused(IntPtr thisPtr, bool paused);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFrameDelta(IntPtr thisPtr);
	}

	/** @} */
}

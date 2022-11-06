//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Settings that control the film grain effect. Film grains adds a time-varying noise effect over the entire image.
	/// </summary>
	[ShowInInspector]
	public partial class FilmGrainSettings : ScriptObject
	{
		private FilmGrainSettings(bool __dummy0) { }

		public FilmGrainSettings()
		{
			Internal_FilmGrainSettings(this);
		}

		/// <summary>Enables or disables the effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_GetEnabled(mCachedPtr); }
			set { Internal_SetEnabled(mCachedPtr, value); }
		}

		/// <summary>Controls how intense are the displayed film grains.</summary>
		[ShowInInspector]
		[Range(0f, 100f, false)]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_GetIntensity(mCachedPtr); }
			set { Internal_SetIntensity(mCachedPtr, value); }
		}

		/// <summary>Controls at what speed do the film grains change.</summary>
		[ShowInInspector]
		[Range(0f, 100f, false)]
		[NativeWrapper]
		public float Speed
		{
			get { return Internal_GetSpeed(mCachedPtr); }
			set { Internal_SetSpeed(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_FilmGrainSettings(FilmGrainSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetIntensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIntensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpeed(IntPtr thisPtr, float value);
	}

	/** @} */
}

//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>Controls how intense are the displayed film grains.</summary>
		[ShowInInspector]
		[Range(0f, 100f, false)]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_getintensity(mCachedPtr); }
			set { Internal_setintensity(mCachedPtr, value); }
		}

		/// <summary>Controls at what speed do the film grains change.</summary>
		[ShowInInspector]
		[Range(0f, 100f, false)]
		[NativeWrapper]
		public float Speed
		{
			get { return Internal_getspeed(mCachedPtr); }
			set { Internal_setspeed(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_FilmGrainSettings(FilmGrainSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getintensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setintensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getspeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setspeed(IntPtr thisPtr, float value);
	}

	/** @} */
}

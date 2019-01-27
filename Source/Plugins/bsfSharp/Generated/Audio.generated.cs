//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Audio
	 *  @{
	 */

	/// <summary>Provides global functionality relating to sounds and music.</summary>
	[ShowInInspector]
	public partial class Audio : ScriptObject
	{
		private Audio(bool __dummy0) { }
		protected Audio() { }

		/// <summary>Determines global audio volume. In range [0, 1].</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static float Volume
		{
			get { return Internal_getVolume(); }
			set { Internal_setVolume(value); }
		}

		/// <summary>Determines if audio reproduction is paused globally.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static bool Paused
		{
			get { return Internal_isPaused(); }
			set { Internal_setPaused(value); }
		}

		/// <summary>Determines the device on which is the audio played back on.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static AudioDevice ActiveDevice
		{
			get
			{
				AudioDevice temp;
				Internal_getActiveDevice(out temp);
				return temp;
			}
			set { Internal_setActiveDevice(ref value); }
		}

		/// <summary>Returns the default audio device identifier.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static AudioDevice DefaultDevice
		{
			get
			{
				AudioDevice temp;
				Internal_getDefaultDevice(out temp);
				return temp;
			}
		}

		/// <summary>Returns a list of all available audio devices.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static AudioDevice[] AllDevices
		{
			get { return Internal_getAllDevices(); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setVolume(float volume);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getVolume();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPaused(bool paused);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isPaused();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setActiveDevice(ref AudioDevice device);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getActiveDevice(out AudioDevice __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getDefaultDevice(out AudioDevice __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AudioDevice[] Internal_getAllDevices();
	}

	/** @} */
}

//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /// <summary>
    /// Identifier for a device that can be used for playing audio.
    /// </summary>
    public class AudioDevice : ScriptObject
    {
        /// <summary>
        /// Constructor for internal use by the runtime only.
        /// </summary>
        private AudioDevice() { }

        /// <summary>
        /// Name of the audio device.
        /// </summary>
        public string Name
        {
            get { return Internal_GetName(mCachedPtr); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetName(IntPtr thisPtr);
    }

    /// <summary>
    /// Provides global functionality relating to sounds and music.
    /// </summary>
    public static class Audio
    {
        /// <summary>
        /// Global audio volume. In range [0, 1].
        /// </summary>
        public static float Volume
        {
            get { return Internal_GetVolume(); }
            set { Internal_SetVolume(value); }
        }

        /// <summary>
        /// Device on which is the audio played back on.
        /// </summary>
        public static AudioDevice ActiveDevice
        {
            get { return Internal_GetActiveDevice(); }
            set
            {
                IntPtr devicePtr = IntPtr.Zero;
                if (value != null)
                    devicePtr = value.GetCachedPtr();

                Internal_SetActiveDevice(devicePtr);
            }
        }

        /// <summary>
        /// Returns the default audio device identifier.
        /// </summary>
        public static AudioDevice DefaultDevice
        {
            get { return Internal_GetDefaultDevice(); }
        }

        /// <summary>
        /// Returns a list of all available audio devices
        /// </summary>
        public static AudioDevice[] AllDevices
        {
            get { return Internal_GetAllDevices(); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVolume(float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetVolume();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetActiveDevice(IntPtr device);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AudioDevice Internal_GetActiveDevice();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AudioDevice Internal_GetDefaultDevice();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AudioDevice[] Internal_GetAllDevices();
    }
}

//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @cond INTEROP */
    /** @addtogroup Interop
     *  @{
     */

    /// <summary>
    /// Wrapper around the native AudioSource class.
    /// <see cref="AudioSource"/>
    /// </summary>
    internal class NativeAudioSource : ScriptObject
    {
        public NativeAudioSource()
        {
            Internal_CreateInstance(this);
        }

        public Vector3 Position
        {
            set { Internal_SetPosition(mCachedPtr, ref value); }
        }

        public Vector3 Velocity
        {
            set { Internal_SetVelocity(mCachedPtr, ref value); }
        }

        public AudioClip Clip
        {
            set
            {
                IntPtr clipPtr = IntPtr.Zero;
                if (value != null)
                    clipPtr = value.GetCachedPtr();

                Internal_SetClip(mCachedPtr, clipPtr);
            }
        }

        public float Volume
        {
            set { Internal_SetVolume(mCachedPtr, value); }
        }

        public float Pitch
        {
            set { Internal_SetPitch(mCachedPtr, value); }
        }

        public bool Loop
        {
            set { Internal_SetIsLooping(mCachedPtr, value); }
        }

        public uint Priority
        {
            set { Internal_SetPriority(mCachedPtr, value); }
        }

        public float MinDistance
        {
            set { Internal_SetMinDistance(mCachedPtr, value); }
        }

        public float Attenuation
        {
            set { Internal_SetAttenuation(mCachedPtr, value); }
        }

        public float Time
        {
            get { return Internal_GetTime(mCachedPtr); }
            set { Internal_SetTime(mCachedPtr, value); }
        }

        public AudioSourceState State
        {
            get { return (AudioSourceState)Internal_GetState(mCachedPtr); }
        }

        public void Play()
        {
            Internal_Play(mCachedPtr);
        }

        public void Pause()
        {
            Internal_Pause(mCachedPtr);
        }

        public void Stop()
        {
            Internal_Stop(mCachedPtr);
        }

        public void Destroy()
        {
            Internal_Destroy(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(NativeAudioSource instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Destroy(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPosition(IntPtr thisPtr, ref Vector3 pos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVelocity(IntPtr thisPtr, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetClip(IntPtr thisPtr, IntPtr clip);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVolume(IntPtr thisPtr, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPitch(IntPtr thisPtr, float pitch);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetIsLooping(IntPtr thisPtr, bool loop);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPriority(IntPtr thisPtr, uint priority);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMinDistance(IntPtr thisPtr, float distance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAttenuation(IntPtr thisPtr, float attenuation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTime(IntPtr thisPtr, float position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetTime(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Play(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Pause(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Stop(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetState(IntPtr thisPtr);
    }

    /** @} */
    /** @endcond */
}

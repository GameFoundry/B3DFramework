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
    /// Wrapper around the native AudioListener class.
    /// <see cref="AudioListener"/>
    /// </summary>
    internal class NativeAudioListener : ScriptObject
    {
        public NativeAudioListener()
        {
            Internal_CreateInstance(this);
        }

        public Vector3 Position
        {
            set { Internal_SetPosition(mCachedPtr, ref value); }
        }

        public Vector3 Direction
        {
            set { Internal_SetDirection(mCachedPtr, ref value); }
        }

        public Vector3 Up
        {
            set { Internal_SetUp(mCachedPtr, ref value); }
        }

        public Vector3 Velocity
        {
            set { Internal_SetVelocity(mCachedPtr, ref value); }
        }

        public void Destroy()
        {
            Internal_Destroy(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(NativeAudioListener instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Destroy(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPosition(IntPtr thisPtr, ref Vector3 pos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDirection(IntPtr thisPtr, ref Vector3 direction);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetUp(IntPtr thisPtr, ref Vector3 up);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVelocity(IntPtr thisPtr, ref Vector3 velocity);
    }

    /** @} */
    /** @endcond */
}

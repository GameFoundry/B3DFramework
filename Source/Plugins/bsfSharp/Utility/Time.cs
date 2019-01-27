//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// Manages all time related functionality.
    /// </summary>
    public static class Time
    {
        /// <summary>
        /// Gets the time elapsed since application start, in seconds. Only gets updated once per frame and keeps ticking
        /// even if the game is not running.
        /// </summary>
        public static float RealElapsed
        {
            get { return Internal_GetRealElapsed(); }
        }

        /// <summary>
        /// Gets the time elapsed since game start, in seconds. This value gets reset any time the game is started, and
        /// will not be updated while the game is paused.
        /// </summary>
        public static float Elapsed
        {
            get { return Internal_GetElapsed(); }
        }

        /// <summary>
        /// Gets the time since last frame was executed, in seconds. Only gets updated once per frame.
        /// </summary>
        public static float FrameDelta
        {
            get { return Internal_GetFrameDelta(); }
        }

        /// <summary>
        /// Returns the sequential index of the current frame. First frame is 0.
        /// </summary>
        public static UInt64 FrameIdx
        {
            get { return Internal_GetFrameNumber(); }
        }

        /// <summary>
        /// Returns the precise time since application start, in microseconds. Unlike other time methods this is 
        /// not only updated every frame, but will return exact time at the moment it is called.
        /// </summary>
        public static UInt64 Precise
        {
            get { return Internal_GetPrecise(); }
        }

        /// <summary>
        /// Multiply to convert microseconds to seconds.
        /// </summary>
        public const float MicroToSecond = 1.0f/1000000.0f;

        /// <summary>
        /// Multiply to convert seconds to microseconds.
        /// </summary>
        public const float SecondToMicro = 1000000.0f;

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRealElapsed();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetElapsed();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetFrameDelta();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Internal_GetFrameNumber();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Internal_GetPrecise();
    }

    /** @} */
}

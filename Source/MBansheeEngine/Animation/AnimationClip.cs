//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Animation
     *  @{
     */

    /// <summary>
    /// Contains animation curves for translation/rotation/scale of scene object/skeleton bones, as well as curves for
    /// generic property animation.
    /// </summary>
    public class AnimationClip : Resource
    {
        // Constructor for runtime use only (dummy parameter to differentiate from the normal constructor)
        private AnimationClip(bool dummy)
        { }

        /// <summary>
        /// Creates a new animation clip with no curves or events.
        /// </summary>
        public AnimationClip()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Returns the length of the animation clip, in seconds.
        /// </summary>
        public float Length
        {
            get { return Internal_GetLength(mCachedPtr); }
        }

        /// <summary>
        /// Returns the number of samples per second the animation clip curves were sampled at. This value is not used by
        /// the animation clip or curves directly since unevenly spaced keyframes are supported. But it can be of value when
        /// determining the original sample rate of an imported animation or similar.
        /// </summary>
        public int SampleRate
        {
            get { return Internal_GetSampleRate(mCachedPtr); }
            set { Internal_SetSampleRate(mCachedPtr, value); }
        }

        /// <summary>
        /// A set of all curves stored in the animation clip.
        /// </summary>
        public AnimationCurves Curves
        {
            get { return Internal_GetAnimationCurves(mCachedPtr); }
            set { Internal_SetAnimationCurves(mCachedPtr, value); }
        }

        /// <summary>
        /// A set of all events stored in the animation clip.
        /// </summary>
        public AnimationEvent[] Events
        {
            get { return Internal_GetAnimationEvents(mCachedPtr); }
            set { Internal_SetAnimationEvents(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(AnimationClip instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AnimationCurves Internal_GetAnimationCurves(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAnimationCurves(IntPtr thisPtr, AnimationCurves curves);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AnimationEvent[] Internal_GetAnimationEvents(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAnimationEvents(IntPtr thisPtr, AnimationEvent[] events);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetLength(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetSampleRate(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetSampleRate(IntPtr thisPtr, int sampleRate);
    }

    /// <summary>
    /// Event that is triggered when animation reaches a certain point.
    /// </summary>
    public class AnimationEvent
    {
        /// <summary>
        /// Constructs a new animation event.
        /// </summary>
        /// <param name="name">Name used to identify the event when it is triggered.</param>
        /// <param name="time">Time at which to trigger the event, in seconds.</param>
        public AnimationEvent(string name, float time)
        {
            Name = name;
            Time = time;
        }

        /// <summary>
        /// Name used to identify the event when it is triggered.
        /// </summary>
        public string Name;

        /// <summary>
        /// Time at which to trigger the event, in seconds.
        /// </summary>
        public float Time;
    }

    /** @} */
}

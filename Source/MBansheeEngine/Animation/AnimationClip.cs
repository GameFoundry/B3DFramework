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
        /// <summary>
        /// A set of all curves stored in the animation clip.
        /// </summary>
        public AnimationCurves Curves
        {
            get { return Internal_GetAnimationCurves(mCachedPtr); }
            set { Internal_SetAnimationCurves(mCachedPtr, value); }
        }

        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        private AnimationClip()
        { }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AnimationCurves Internal_GetAnimationCurves(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAnimationCurves(IntPtr thisPtr, AnimationCurves curves);
    }

    /** @} */
}

//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Animation
     *  @{
     */

    /// <summary>
    /// Contains a set of animation curves used within an <see cref="AnimationClip"/>.
    /// </summary>
    public class AnimationCurves
    {
        /// <summary>
        /// Curves for animating a scene object's position.
        /// </summary>
        public NamedVector3Curve[] PositionCurves;

        /// <summary>
        /// Curves for animating a scene object's rotation (in euler angles).
        /// </summary>
        public NamedVector3Curve[] RotationCurves;

        /// <summary>
        /// Curves for animating a scene object's scale.
        /// </summary>
        public NamedVector3Curve[] ScaleCurves;

        /// <summary>
        /// Curves for animating generic component properties.
        /// </summary>
        public NamedFloatCurve[] FloatCurves;
    }

    /** @} */
}
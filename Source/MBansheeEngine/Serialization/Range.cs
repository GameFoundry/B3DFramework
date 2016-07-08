//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Makes an integer or a floating point field be displayed as a slider with a specified range in the inspector.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field)]
    public sealed class Range : Attribute
    {
        /// <summary>
        /// Creates a new range attribute.
        /// </summary>
        /// <param name="min">Minimum boundary of the range to clamp the field value to.</param>
        /// <param name="max">Maximum boundary of the range to clamp the field value to.</param>
        /// <param name="step">Minimum change of the field value every change should be multiple of.</param>
        /// 
        public Range(float min, float max, float step=0f)
        {
            this.min = min;
            this.max = max;
            this.step = step;
        }

        private float min;
        private float max;
        private float step;
    }

    /** @} */
}

//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

namespace bs
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Makes an integer or a floating point field be displayed as a slider with a specified range in the inspector.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public sealed class Range : Attribute
    {
        /// <summary>
        /// Creates a new range attribute.
        /// </summary>
        /// <param name="min">Minimum boundary of the range to clamp the field value to.</param>
        /// <param name="max">Maximum boundary of the range to clamp the field value to.</param>
        /// <param name="slider">Whether the field should be rendered as a slider.</param>
        public Range(float min, float max, bool slider = true)
        {
            this.min = min;
            this.max = max;
            this.slider = slider;
        }

        #pragma warning disable 0414
        private float min;
        private float max;
        private bool slider;
        #pragma warning restore 0414
    }

    /** @} */
}

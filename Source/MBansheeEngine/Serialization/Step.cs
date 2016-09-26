//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Makes an integer or a floating point field vary by multiples of the specified value.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field)]
    public sealed class Step : Attribute
    {
        /// <summary>
        /// Creates a new Step attribute.
        /// </summary>
        /// <param name="step">Minimum change of the field. Every change will be rounded to a multiple of this value.</param>
        public Step(float step)
        {
            this.step = step;
        }

        private float step;
    }
}

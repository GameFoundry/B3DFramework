//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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

        #pragma warning disable 0414
        private float step;
        #pragma warning restore 0414
    }

    /** @} */
}

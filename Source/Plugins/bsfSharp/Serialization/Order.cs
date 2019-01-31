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
    /// Specifies the order of a field or a property relative to other fields/properties in the same type. The
    /// order determines in what order are fields listed for serialization or inspection purposes.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public sealed class Order : Attribute
    {
        /// <summary>
        /// Creates a new order attribute.
        /// </summary>
        /// <param name="index">Index that determines the order. Smaller number means the field/property will be ordered
        /// before elements with a higher order index.</param>
        public Order(int index)
        {
            this.index = index;
        }

#pragma warning disable 0414
        private int index;
#pragma warning restore 0414
    }

    /** @} */
}

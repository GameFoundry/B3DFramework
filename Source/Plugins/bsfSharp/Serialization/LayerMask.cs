//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Changes the editor inspector GUI element of a 64-bit integer to a layer-mask selection drop down. 
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public sealed class LayerMask : Attribute
    {
        /// <summary>
        /// Creates a new layer-mask attribute.
        /// </summary>
        public LayerMask()
        { }
    }

    /** @} */
}

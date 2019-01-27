//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Attribute that may be used on fields or properties to mark them as serializable. Normally public fields are 
    /// serialized by default, and you can use this attribute to mark private/protected/internal fields as serialized, or to
    /// mark properties (never serialized by default) as serializable as well. Member must be in a serializable object
    /// otherwise this attribute has no effect, and the type of the field/property must be a serializable type.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public sealed class SerializeField : Attribute
    {
    }

    /** @} */
}

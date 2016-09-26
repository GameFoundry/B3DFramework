//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

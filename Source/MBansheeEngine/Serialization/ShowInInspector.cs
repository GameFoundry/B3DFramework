//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Attribute that forces a field or a property to be visible in the inspector window in editor. Normally public fields
    /// are show in the inspector by default, and you can use this attribute to show private/protected/internal fields, or
    /// to show properties (never shown by default) as well. Member must be in a serializable object otherwise this
    /// attribute has no effect, and the type of the field/property must be a serializable type.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public sealed class ShowInInspector : Attribute
    {
    }

    /** @} */
}
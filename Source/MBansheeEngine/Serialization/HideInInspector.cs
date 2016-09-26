//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Attribute that prevents a field or a property from being visible in the inspector window in editor. Normally those
    /// fields are public fields of a <see cref="Component"/>, <see cref="Resource"/> or a class marked with a 
    /// <see cref="SerializeObject"/> attribute.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public sealed class HideInInspector : Attribute
    {
    }

    /** @} */
}
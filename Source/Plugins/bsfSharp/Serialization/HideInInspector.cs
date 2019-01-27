//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
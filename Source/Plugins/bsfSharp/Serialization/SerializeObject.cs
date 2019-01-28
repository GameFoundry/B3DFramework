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
    /// Forces a class or a struct to be serializable. Normally only Component and Resource types are serialized
    /// but you can use this attribute to force arbitrary types to be serialized.
    /// </summary>
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct)]
    public sealed class SerializeObject : Attribute
    {
    }

    /** @} */
}

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
    /// Attribute that signifies that a field or a property passes its value by copy. This lets the system (such as the
    /// inspector window) know that the value needs to be re-assigned to the field/property if any changes to it are made.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public sealed class PassByCopy : Attribute { }

    /** @} */
}

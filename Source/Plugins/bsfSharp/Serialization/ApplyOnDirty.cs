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
    /// Attribute that signifies if a property changes those changes might not be recognized by the parent object until
    /// the property setter is called. This lets the system (such as the inspector window) know that the value needs to be
    /// re-assigned to the property if any changes to it are made.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property)]
    public sealed class ApplyOnDirty : Attribute { }

    /** @} */
}

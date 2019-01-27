//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// When a quaternion is displayed in the inspector, by default it will be displayed as converted into euler angles.
    /// Use this attribute to force it to be displayed as a quaternion (4D value) with no conversion instead.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field)]
    public sealed class AsQuaternion : Attribute { }

    /** @} */
}

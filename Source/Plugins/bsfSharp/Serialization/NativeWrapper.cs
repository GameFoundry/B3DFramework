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
    /// Attribute that signifies that property is a wrapper to some native value. It's getter/setter calls are native
    /// interop calls which then modify the internal value.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property)]
    public sealed class NativeWrapper : Attribute { }

    /** @} */
}

//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Scene
     *  @{
     */

    /// <summary>
    /// Attribute that allows Component's code to be executed while game is not running in the editor. Normally the
    /// initialize/update/destroy methods will not be running while the game is paused or not running, but this overrides
    /// that behaviour.
    /// </summary>
    [AttributeUsage(AttributeTargets.Class)]
    public sealed class RunInEditor : Attribute
    {
    }

    /** @} */
}
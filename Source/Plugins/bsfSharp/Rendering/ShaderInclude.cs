//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Rendering
     *  @{
     */

    /// <summary>
    /// Contains code and definitions that are shared between <see cref="Shader"/> resources.
    /// </summary>
    public class ShaderInclude : Resource
    {
        /// <summary>
        /// Constuctor for internal runtime use only.
        /// </summary>
        private ShaderInclude()
        { }
    }

    /** @} */
}

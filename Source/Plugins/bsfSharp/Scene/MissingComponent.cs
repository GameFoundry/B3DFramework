//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
    /// Component type that is used in place of deserialized component types that no longer exist.
    /// </summary>
    internal sealed class MissingComponent : ManagedComponent
    {
    }

    /** @} */
}

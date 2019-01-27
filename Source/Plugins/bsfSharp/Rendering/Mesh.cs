//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Rendering
     *  @{
     */

    public partial class Mesh
    {
        /// <summary>
        /// Returns local bounds of the geometry contained in the vertex buffers for all sub-meshes.
        /// </summary>
        public Bounds Bounds
        {
            get
            {
                AABox box;
                Sphere sphere;

                Internal_getBounds(mCachedPtr, out box, out sphere);

                return new Bounds(box, sphere);
            }
        }
    }

    /** @} */
}

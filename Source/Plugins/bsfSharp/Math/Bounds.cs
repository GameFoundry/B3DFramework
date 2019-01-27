//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// Bounds represented by an axis aligned box and a sphere.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Bounds
    {
        /// <summary>
        /// Creates new bounds.
        /// </summary>
        /// <param name="box">Axis aligned box representation of the bounds.</param>
        /// <param name="sphere">Sphere representation of the bounds.</param>
        public Bounds(AABox box, Sphere sphere)
        {
            Box = box;
            Sphere = sphere;
        }

        /// <summary>
        /// Axis aligned box representation of the bounds.
        /// </summary>
        public AABox Box;

        /// <summary>
        /// Sphere representation of the bounds.
        /// </summary>
        public Sphere Sphere;
    }

    /** @} */
}

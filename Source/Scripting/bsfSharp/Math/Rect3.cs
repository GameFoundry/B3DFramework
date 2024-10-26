//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Math
     *  @{
     */

    public partial struct Rect3
    {
        /// <summary>
        /// Creates a new rectangle.
        /// </summary>
        /// <param name="center">Origin of the rectangle. </param>
        /// <param name="axes">Two axes that define orientation of the rectangle. Axes extend from the origin. Axes should
        ///                    be normalized.</param>
        /// <param name="extents">Two extents that define the size of the rectangle. Extends should be half the width/height
        ///                       as they are applied in both directions.</param>
        public Rect3(Vector3 center, Vector3[] axes, float[] extents)
        {
            Center = center;
            HorizontalAxis = axes[0];
            VerticalAxis = axes[1];
            HorizontalExtent = extents[0];
            VerticalExtent = extents[1];
        }
    };

    /** @} */
}

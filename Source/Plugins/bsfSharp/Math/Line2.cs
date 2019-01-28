//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// A line in 2D space represented with an origin and direction.
    /// </summary>
    public class Line2
    {
        public Line2() { }

        public Line2(Vector2 origin, Vector2 direction)
        {
            this.origin = origin;
            this.direction = direction;
        }

        /// <summary>
        /// Line/Line intersection, returns boolean result and distance to intersection point.
        /// </summary>
        /// <returns>True on intersection, false otherwise.</returns>
        public bool Intersects(Line2 rhs, out float t)
        {
            t = 0.0f;

            Vector2 diff = rhs.origin - origin;

            float rxs = Vector2.Cross(direction, rhs.direction);
            float qpxr = Vector2.Cross(diff, direction);

            if (rxs == 0.0f && qpxr == 0.0f)
                return false;

            if (rxs == 0.0f && qpxr != 0.0f)
                return false;

            t = Vector2.Cross(diff, rhs.direction) / rxs;
            var u = Vector2.Cross(diff, direction) / rxs;

            if (rxs != 0.0f && (0 <= t && t <= 1) && (0 <= u && u <= 1))
                return true;

            return false;
        }

        public Vector2 origin = new Vector2(0.0f, 0.0f);
        public Vector2 direction = new Vector2(0.0f, 1.0f);
    }

    /** @} */
}

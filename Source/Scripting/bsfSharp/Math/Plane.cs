//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Math
     *  @{
     */

    public partial struct Plane // Note: Must match C++ class Plane
    {
        /// <summary>
        /// Creates a plane from a normal and a distance from the plane.
        /// </summary>
        /// <param name="normal">Plane normal pointing in the positive half-space.</param>
        /// <param name="d">Distance of the plane from the origin, along the normal.</param>
        public Plane(Vector3 normal, float d)
        {
            this.Normal = normal;
            this.D = d;
        }

        /// <summary>
        /// Creates a plane from three points on the plane. The points must not be colinear.
        /// </summary>
        /// <param name="a">A point in the plane.</param>
        /// <param name="b">A point in the plane.</param>
        /// <param name="c">A point in the plane.</param>
        public Plane(Vector3 a, Vector3 b, Vector3 c)
        {
            Vector3 e0 = b - a;
            Vector3 e1 = c - a;
            Normal = Vector3.Cross(e0, e1);
            Normal.Normalize();

            D = Vector3.Dot(Normal, a);
        }

        /// <summary>
        /// Returns a distance from point to plane.
        /// </summary>
        /// <param name="point">Point to test.</param>
        /// <returns>
        /// Distance to the plane. Will be positive if the point is on the positive side of the plane, negative if on the
        /// negative side of the plane, or zero if the point is on the plane.
        /// </returns>
        float GetDistance(Vector3 point)
        {
            return Vector3.Dot(Normal, point) - D;
        }

        /// <summary>
        /// Returns the side of the plane where the point is located on. No side signifies the point is on the plane.
        /// </summary>
        /// <param name="point">Point to test.</param>
        /// <param name="epsilon">Extra depth of the plane to help with precision issues.</param>
        /// <returns>Side on the plane the point is located on.</returns>
        public PlaneSide GetSide(Vector3 point, float epsilon = 0.0f)
        {
            float dist = GetDistance(point);

            if (dist > epsilon)
                return PlaneSide.Positive;

            if (dist < -epsilon)
                return PlaneSide.Negative;

            return PlaneSide.None;
        }
    }

    /** @} */
}

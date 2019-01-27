//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// A plane represented by a normal and a distance.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Plane // Note: Must match C++ class Plane
    {
        /// <summary>
        /// The "positive side" of the plane is the half space to which the plane normal points. The "negative side" is the 
        /// other half space. The flag "no side" indicates the plane itself.
        /// </summary>
        public enum Side
        {
            None,
            Positive,
            Negative,
            Both
        };

        public Vector3 normal;
        public float d;

        /// <summary>
        /// Creates a plane from a normal and a distance from the plane.
        /// </summary>
        /// <param name="normal">Plane normal pointing in the positive half-space.</param>
        /// <param name="d">Distance of the plane from the origin, along the normal.</param>
        public Plane(Vector3 normal, float d)
        {
            this.normal = normal;
            this.d = d;
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
            normal = Vector3.Cross(e0, e1);
            normal.Normalize();

            d = Vector3.Dot(normal, a);
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
            return Vector3.Dot(normal, point) - d;
        }

        /// <summary>
        /// Returns the side of the plane where the point is located on. No side signifies the point is on the plane.
        /// </summary>
        /// <param name="point">Point to test.</param>
        /// <param name="epsilon">Extra depth of the plane to help with precision issues.</param>
        /// <returns>Side on the plane the point is located on.</returns>
        public Side GetSide(Vector3 point, float epsilon = 0.0f)
        {
            float dist = GetDistance(point);

            if (dist > epsilon)
                return Side.Positive;

            if (dist < -epsilon)
                return Side.Negative;

            return Side.None;
        }
    }

    /** @} */
}

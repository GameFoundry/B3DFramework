//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// A ray in 3D space represented with an origin and direction.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Ray // Note: Must match C++ struct Ray
    {
        public Vector3 origin;
        public Vector3 direction;

        /// <summary>
        /// Creates a new ray.
        /// </summary>
        /// <param name="origin">Coordinates for the origin of the ray.</param>
        /// <param name="direction">Normalized direction of the ray.</param>
        public Ray(Vector3 origin, Vector3 direction)
        {
            this.origin = origin;
            this.direction = direction;
        }

        /// <summary>
        /// Multiples ray by a scalar and retrieves a point along the ray.
        /// </summary>
        /// <param name="ray">Ray to transform.</param>
        /// <param name="t">How far along the ray to retrieve the point.</param>
        /// <returns>Point along the ray <paramref name="t"/> units away from the origin.</returns>
        public static Vector3 operator*(Ray ray, float t) 
        {
            return ray.origin + ray.direction * t;
        }

        /// <summary>
        /// Transforms the ray by the specified matrix. If the matrix is affine use
        /// <see cref="TransformAffine"/> as it is faster.
        /// </summary>
        /// <param name="matrix">Matrix to transform the ray by.</param>
        public void Transform(Matrix4 matrix)
        {
            Vector3 end = this * 1.0f;

            origin = matrix.Multiply(origin);
            end = matrix.Multiply(end);

            direction = Vector3.Normalize(end - origin);
        }

        /// <summary>
        /// Transforms the ray by the specified affine matrix.
        /// </summary>
        /// <param name="matrix">Affine matrix to transform the ray by.</param>
        public void TransformAffine(Matrix4 matrix)
        {
            Vector3 end = this * 1.0f;

            origin = matrix.MultiplyDirection(origin);
            end = matrix.MultiplyDirection(end);

            direction = Vector3.Normalize(end - origin);
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return String.Format("(origin: {0} direction: {1})", origin, direction);
        }
    };

    /** @} */
}

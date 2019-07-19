//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// A four dimensional vector with a homogeneous w coordinate.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Vector4
    {
        public static readonly Vector4 Zero = new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        public static readonly Vector4 One = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        public static readonly Vector4 XAxis = new Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        public static readonly Vector4 YAxis = new Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        public static readonly Vector4 ZAxis = new Vector4(0.0f, 0.0f, 1.0f, 0.0f);

        public float x;
        public float y;
        public float z;
        public float w;

        /// <summary>
        /// Accesses a specific component of the vector.
        /// </summary>
        /// <param name="index">Index of the component.</param>
        /// <returns>Value of the specific component.</returns>
        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    case 3:
                        return w;
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector4 index.");
                }
            }

            set
            {
                switch (index)
                {
                    case 0:
                        x = value;
                        break;
                    case 1:
                        y = value;
                        break;
                    case 2:
                        z = value;
                        break;
                    case 3:
                        w = value;
                        break;
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector4 index.");
                }
            }
        }

        /// <summary>
        /// Returns a normalized copy of the vector.
        /// </summary>
        public Vector4 Normalized
        {
            get
            {
                return Normalize(this);
            }
        }

        /// <summary>
        /// Returns the length of the vector.
        /// </summary>
        public float Length
        {
            get
            {
                return MathEx.Sqrt(x * x + y * y + z * z + w * w);
            }
        }

        /// <summary>
        /// Returns the squared length of the vector.
        /// </summary>
        public float SqrdLength
        {
            get
            {
                return (x * x + y * y + z * z + w * w);
            }
        }

        /// <summary>
        /// Creates a new default initialized vector value.
        /// </summary>
        public static Vector4 Default()
        {
            return new Vector4();
        }

        /// <summary>
        /// Creates a new four dimensional vector.
        /// </summary>
        /// <param name="x">X coordinate.</param>
        /// <param name="y">Y coordinate.</param>
        /// <param name="z">Z coordinate.</param>
        /// <param name="w">Homogeneous W coordinate.</param>
        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        /// <summary>
        /// Converts a homogenous vector into a three dimensional vector. w component is discarded.
        /// </summary>
        /// <param name="vec">Vector to convert.</param>
        /// <returns>A new three dimensional vector.</returns>
        public static explicit operator Vector3(Vector4 vec)
        {
            return new Vector3(vec.x, vec.y, vec.z);
        }

        public static Vector4 operator+ (Vector4 a, Vector4 b)
        {
            return new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        public static Vector4 operator- (Vector4 a, Vector4 b)
        {
            return new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        }

        public static Vector4 operator- (Vector4 v)
        {
            return new Vector4(-v.x, -v.y, -v.z, -v.w);
        }

        public static Vector4 operator *(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        }

        public static Vector4 operator* (Vector4 v, float d)
        {
            return new Vector4(v.x * d, v.y * d, v.z * d, v.w * d);
        }

        public static Vector4 operator* (float d, Vector4 v)
        {
            return new Vector4(v.x * d, v.y * d, v.z * d, v.w * d);
        }

        public static Vector4 operator /(Vector4 v, float d)
        {
            return new Vector4(v.x / d, v.y / d, v.z / d, v.w / d);
        }

        public static bool operator== (Vector4 lhs, Vector4 rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
        }

        public static bool operator!= (Vector4 lhs, Vector4 rhs)
        {
            return !(lhs == rhs);
        }

        /// <summary>
        /// Scales one vector by another.
        /// </summary>
        /// <param name="a">First four dimensional vector.</param>
        /// <param name="b">Second four dimensional vector.</param>
        /// <returns>One vector scaled by another.</returns>
        public static Vector4 Scale(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        }

        /// <summary>
        /// Normalizes the provided vector and returns the normalized copy.
        /// </summary>
        /// <param name="value">Vector to normalize.</param>
        /// <returns>Normalized copy of the vector.</returns>
        public static Vector4 Normalize(Vector4 value)
        {
            float sqrdLen = value.SqrdLength;
            if (sqrdLen > 1e-04f)
                return value * MathEx.InvSqrt(sqrdLen);

            return value;
        }

        /// <summary>
        /// Calculates the inner product of the two vectors.
        /// </summary>
        /// <param name="lhs">First four dimensional vector.</param>
        /// <param name="rhs">Second four dimensional vector.</param>
        /// <returns>Inner product between the two vectors.</returns>
        public static float Dot(Vector4 lhs, Vector4 rhs)
        {
            return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
        }

        /// <summary>
        /// Calculates the distance between two points.
        /// </summary>
        /// <param name="a">First four dimensional point.</param>
        /// <param name="b">Second four dimensional point.</param>
        /// <returns>Distance between the two points.</returns>
        public static float Distance(Vector4 a, Vector4 b)
        {
            Vector4 vector4 = new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
            return MathEx.Sqrt(vector4.x * vector4.x + vector4.y * vector4.y + vector4.z * vector4.z + vector4.w * vector4.w);
        }

        /// <summary>
        /// Calculates the magnitude of the provided vector.
        /// </summary>
        /// <param name="v">Vector to calculate the magnitude for.</param>
        /// <returns>Magnitude of the vector.</returns>
        public static float Magnitude(Vector4 v)
        {
            return MathEx.Sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
        }

        /// <summary>
        /// Calculates the squared magnitude of the provided vector.
        /// </summary>
        /// <param name="v">Vector to calculate the magnitude for.</param>
        /// <returns>Squared magnitude of the vector.</returns>
        public static float SqrMagnitude(Vector4 v)
        {
            return (v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
        }

        /// <summary>
        /// Scales the components of the vector by specified scale factors.
        /// </summary>
        /// <param name="scale">Scale factors to multiply components by.</param>
        public void Scale(Vector4 scale)
        {
            x *= scale.x;
            y *= scale.y;
            z *= scale.z;
            w *= scale.w;
        }

        /// <summary>
        /// Normalizes the vector.
        /// </summary>
        public void Normalize()
        {
            float sqrdLen = SqrdLength;
            if (sqrdLen > 1e-04f)
                this = this * MathEx.InvSqrt(sqrdLen);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode() << 2 ^ z.GetHashCode() >> 2 ^ w.GetHashCode() >> 1;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Vector4))
                return false;

            Vector4 vec = (Vector4)other;
            if (x.Equals(vec.x) && y.Equals(vec.y) && z.Equals(vec.z) && w.Equals(vec.w))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return "(" + x + ", " + y + ", " + z + ", " + w + ")";
        }
    }

    /** @} */
}

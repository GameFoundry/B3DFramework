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
    public partial struct Vector4
    {
        public static readonly Vector4 Zero = new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        public static readonly Vector4 One = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        public static readonly Vector4 XAxis = new Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        public static readonly Vector4 YAxis = new Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        public static readonly Vector4 ZAxis = new Vector4(0.0f, 0.0f, 1.0f, 0.0f);

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
                        return X;
                    case 1:
                        return Y;
                    case 2:
                        return Z;
                    case 3:
                        return W;
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector4 index.");
                }
            }

            set
            {
                switch (index)
                {
                    case 0:
                        X = value;
                        break;
                    case 1:
                        Y = value;
                        break;
                    case 2:
                        Z = value;
                        break;
                    case 3:
                        W = value;
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
                return MathEx.Sqrt(X * X + Y * Y + Z * Z + W * W);
            }
        }

        /// <summary>
        /// Returns the squared length of the vector.
        /// </summary>
        public float SqrdLength
        {
            get
            {
                return (X * X + Y * Y + Z * Z + W * W);
            }
        }

        /// <summary>
        /// Converts a homogenous vector into a three dimensional vector. w component is discarded.
        /// </summary>
        /// <param name="vec">Vector to convert.</param>
        /// <returns>A new three dimensional vector.</returns>
        public static explicit operator Vector3(Vector4 vec)
        {
            return new Vector3(vec.X, vec.Y, vec.Z);
        }

        public static Vector4 operator+ (Vector4 a, Vector4 b)
        {
            return new Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
        }

        public static Vector4 operator- (Vector4 a, Vector4 b)
        {
            return new Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
        }

        public static Vector4 operator- (Vector4 v)
        {
            return new Vector4(-v.X, -v.Y, -v.Z, -v.W);
        }

        public static Vector4 operator *(Vector4 a, Vector4 b)
        {
            return new Vector4(a.X * b.X, a.Y * b.Y, a.Z * b.Z, a.W * b.W);
        }

        public static Vector4 operator* (Vector4 v, float d)
        {
            return new Vector4(v.X * d, v.Y * d, v.Z * d, v.W * d);
        }

        public static Vector4 operator* (float d, Vector4 v)
        {
            return new Vector4(v.X * d, v.Y * d, v.Z * d, v.W * d);
        }

        public static Vector4 operator /(Vector4 v, float d)
        {
            return new Vector4(v.X / d, v.Y / d, v.Z / d, v.W / d);
        }

        public static bool operator== (Vector4 lhs, Vector4 rhs)
        {
            return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z && lhs.W == rhs.W;
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
            return new Vector4(a.X * b.X, a.Y * b.Y, a.Z * b.Z, a.W * b.W);
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
            return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z + lhs.W * rhs.W;
        }

        /// <summary>
        /// Calculates the distance between two points.
        /// </summary>
        /// <param name="a">First four dimensional point.</param>
        /// <param name="b">Second four dimensional point.</param>
        /// <returns>Distance between the two points.</returns>
        public static float Distance(Vector4 a, Vector4 b)
        {
            Vector4 vector4 = new Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
            return MathEx.Sqrt(vector4.X * vector4.X + vector4.Y * vector4.Y + vector4.Z * vector4.Z + vector4.W * vector4.W);
        }

        /// <summary>
        /// Calculates the magnitude of the provided vector.
        /// </summary>
        /// <param name="v">Vector to calculate the magnitude for.</param>
        /// <returns>Magnitude of the vector.</returns>
        public static float Magnitude(Vector4 v)
        {
            return MathEx.Sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z + v.W * v.W);
        }

        /// <summary>
        /// Calculates the squared magnitude of the provided vector.
        /// </summary>
        /// <param name="v">Vector to calculate the magnitude for.</param>
        /// <returns>Squared magnitude of the vector.</returns>
        public static float SqrMagnitude(Vector4 v)
        {
            return (v.X * v.X + v.Y * v.Y + v.Z * v.Z + v.W * v.W);
        }

        /// <summary>
        /// Scales the components of the vector by specified scale factors.
        /// </summary>
        /// <param name="scale">Scale factors to multiply components by.</param>
        public void Scale(Vector4 scale)
        {
            X *= scale.X;
            Y *= scale.Y;
            Z *= scale.Z;
            W *= scale.W;
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
            return X.GetHashCode() ^ Y.GetHashCode() << 2 ^ Z.GetHashCode() >> 2 ^ W.GetHashCode() >> 1;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Vector4))
                return false;

            Vector4 vec = (Vector4)other;
            if (X.Equals(vec.X) && Y.Equals(vec.Y) && Z.Equals(vec.Z) && W.Equals(vec.W))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return "(" + X + ", " + Y + ", " + Z + ", " + W + ")";
        }
    }

    /** @} */
}

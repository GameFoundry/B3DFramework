using System;
using System.Runtime.InteropServices;

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
    /// A three dimensional vector with integer coordinates.
    /// </summary>
    public partial struct Vector3I
    {
        public static readonly Vector3I Zero = new Vector3I(0, 0, 0);
        public static readonly Vector3I One = new Vector3I(1, 1, 1);
        public static readonly Vector3I XAxis = new Vector3I(1, 0, 0);
        public static readonly Vector3I YAxis = new Vector3I(0, 1, 0);
        public static readonly Vector3I ZAxis = new Vector3I(0, 0, 1);

        /// <summary>
        /// Accesses a specific component of the vector.
        /// </summary>
        /// <param name="index">Index of the component.</param>
        /// <returns>Value of the specific component.</returns>
        public int this[int index]
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
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector3I index.");
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
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector3I index.");
                }
            }
        }

        /// <summary>
        /// Returns the squared length of the vector.
        /// </summary>
        public int SqrdLength
        {
            get
            {
                return (x * x + y * y + z * z);
            }
        }

        /// <summary>
        /// Converts a three dimensional vector into a four dimensional vector. w component will be set to zero.
        /// </summary>
        /// <param name="vec">Vector to convert.</param>
        /// <returns>A new four dimensional vector.</returns>
        public static explicit operator Vector4I(Vector3I vec)
        {
            return new Vector4I(vec.x, vec.y, vec.z, 0);
        }

        public static Vector3I operator +(Vector3I a, Vector3I b)
        {
            return new Vector3I(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public static Vector3I operator -(Vector3I a, Vector3I b)
        {
            return new Vector3I(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        public static Vector3I operator -(Vector3I v)
        {
            return new Vector3I(-v.x, -v.y, -v.z);
        }

        public static Vector3I operator *(Vector3I a, Vector3I b)
        {
            return new Vector3I(a.x * b.x, a.y * b.y, a.z * b.z);
        }

        public static Vector3I operator *(Vector3I v, int d)
        {
            return new Vector3I(v.x * d, v.y * d, v.z * d);
        }

        public static Vector3I operator *(int d, Vector3I v)
        {
            return new Vector3I(v.x * d, v.y * d, v.z * d);
        }

        public static Vector3I operator /(Vector3I v, int d)
        {
            return new Vector3I(v.x / d, v.y / d, v.z / d);
        }

        public static bool operator ==(Vector3I lhs, Vector3I rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }

        public static bool operator !=(Vector3I lhs, Vector3I rhs)
        {
            return !(lhs == rhs);
        }

        /// <summary>
        /// Calculates the squared magnitude of the provided vector.
        /// </summary>
        /// <param name="v">Vector to calculate the magnitude for.</param>
        /// <returns>Squared magnitude of the vector.</returns>
        public static int SqrMagnitude(Vector3I v)
        {
            return (v.x * v.x + v.y * v.y + v.z * v.z);
        }

        /// <summary>
        /// Returns the maximum of all the vector components as a new vector.
        /// </summary>
        /// <param name="a">First vector.</param>
        /// <param name="b">Second vector.</param>
        /// <returns>Vector consisting of maximum components of the first and second vector.</returns>
        public static Vector3I Max(Vector3I a, Vector3I b)
        {
            return new Vector3I(MathEx.Max(a.x, b.x), MathEx.Max(a.y, b.y), MathEx.Max(a.z, b.z));
        }

        /// <summary>
        /// Returns the minimum of all the vector components as a new vector.
        /// </summary>
        /// <param name="a">First vector.</param>
        /// <param name="b">Second vector.</param>
        /// <returns>Vector consisting of minimum components of the first and second vector.</returns>
        public static Vector3I Min(Vector3I a, Vector3I b)
        {
            return new Vector3I(MathEx.Min(a.x, b.x), MathEx.Min(a.y, b.y), MathEx.Min(a.z, b.z));
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode() << 2 ^ z.GetHashCode() >> 2;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Vector3I))
                return false;

            Vector3I vec = (Vector3I)other;
            if (x.Equals(vec.x) && y.Equals(vec.y) && z.Equals(vec.z))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return "(" + x + ", " + y + ", " + z + ")";
        }
    }
}

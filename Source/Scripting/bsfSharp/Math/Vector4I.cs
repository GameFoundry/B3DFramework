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
    /// A four dimensional vector with integer coordinates.
    /// </summary>
    public partial struct Vector4I
    {
        public static readonly Vector4I Zero = new Vector4I(0, 0, 0, 0);
        public static readonly Vector4I One = new Vector4I(1, 1, 1, 1);
        public static readonly Vector4I XAxis = new Vector4I(1, 0, 0, 0);
        public static readonly Vector4I YAxis = new Vector4I(0, 1, 0, 0);
        public static readonly Vector4I ZAxis = new Vector4I(0, 0, 1, 0);

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
                    case 3:
                        return w;
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector4I index.");
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
                        throw new IndexOutOfRangeException("Invalid Vector4I index.");
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
                return (x * x + y * y + z * z + w * w);
            }
        }

        /// <summary>
        /// Converts a homogenous vector into a three dimensional vector. w component is discarded.
        /// </summary>
        /// <param name="vec">Vector to convert.</param>
        /// <returns>A new three dimensional vector.</returns>
        public static explicit operator Vector3I(Vector4I vec)
        {
            return new Vector3I(vec.x, vec.y, vec.z);
        }

        public static Vector4I operator +(Vector4I a, Vector4I b)
        {
            return new Vector4I(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        public static Vector4I operator -(Vector4I a, Vector4I b)
        {
            return new Vector4I(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        }

        public static Vector4I operator -(Vector4I v)
        {
            return new Vector4I(-v.x, -v.y, -v.z, -v.w);
        }

        public static Vector4I operator *(Vector4I a, Vector4I b)
        {
            return new Vector4I(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        }

        public static Vector4I operator *(Vector4I v, int d)
        {
            return new Vector4I(v.x * d, v.y * d, v.z * d, v.w * d);
        }

        public static Vector4I operator *(int d, Vector4I v)
        {
            return new Vector4I(v.x * d, v.y * d, v.z * d, v.w * d);
        }

        public static Vector4I operator /(Vector4I v, int d)
        {
            return new Vector4I(v.x / d, v.y / d, v.z / d, v.w / d);
        }

        public static bool operator ==(Vector4I lhs, Vector4I rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
        }

        public static bool operator !=(Vector4I lhs, Vector4I rhs)
        {
            return !(lhs == rhs);
        }

        /// <summary>
        /// Calculates the squared magnitude of the provided vector.
        /// </summary>
        /// <param name="v">Vector to calculate the magnitude for.</param>
        /// <returns>Squared magnitude of the vector.</returns>
        public static int SqrMagnitude(Vector4I v)
        {
            return (v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode() << 2 ^ z.GetHashCode() >> 2 ^ w.GetHashCode() >> 1;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Vector4I))
                return false;

            Vector4I vec = (Vector4I)other;
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
}


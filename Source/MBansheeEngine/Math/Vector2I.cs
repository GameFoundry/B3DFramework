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
    /// A two dimensional vector with integer coordinates.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2I // Note: Must match C++ class Vector2I
    {
        public int x;
        public int y;

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
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector2I index.");
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
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector2I index.");
                }
            }
        }

        /// <summary>
        /// Creates a new two dimensional vector.
        /// </summary>
        /// <param name="x">X coordinate.</param>
        /// <param name="y">Y coordinate.</param>
        public Vector2I(int x, int y)
        {
            this.x = x;
            this.y = y;
        }

        /// <summary>
        /// Returns length of the vector.
        /// </summary>
        public float Length
        {
            get
            {
                return (float)MathEx.Sqrt(x * x + y * y);
            }
        }

        /// <summary>
        /// Returns the squared length of the vector.
        /// </summary>
        public float SqrdLength
        {
            get
            {
                return (x * x + y * y);
            }
        }

        /// <summary>
        /// Returns the manhattan distance between two points.
        /// </summary>
        /// <param name="a">First two dimensional point.</param>
        /// <param name="b">Second two dimensional point.</param>
        /// <returns>Manhattan distance between the two points.</returns>
        public static int Distance(Vector2I a, Vector2I b)
        {
            return Math.Abs(b.x - a.x) + Math.Abs(b.y - a.y);
        }

        public static Vector2I operator +(Vector2I a, Vector2I b)
        {
            return new Vector2I(a.x + b.x, a.y + b.y);
        }

        public static Vector2I operator -(Vector2I a, Vector2I b)
        {
            return new Vector2I(a.x - b.x, a.y - b.y);
        }

        public static Vector2I operator -(Vector2I v)
        {
            return new Vector2I(-v.x, -v.y);
        }

        public static Vector2I operator *(Vector2I v, int d)
        {
            return new Vector2I(v.x * d, v.y * d);
        }

        public static Vector2I operator *(int d, Vector2I v)
        {
            return new Vector2I(v.x * d, v.y * d);
        }

        public static Vector2I operator /(Vector2I v, int d)
        {
            return new Vector2I(v.x / d, v.y / d);
        }

        public static bool operator ==(Vector2I lhs, Vector2I rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }

        public static bool operator !=(Vector2I lhs, Vector2I rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode() << 2;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Vector2I))
                return false;

            Vector2I vec = (Vector2I)other;
            if (x.Equals(vec.x) && y.Equals(vec.y))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return "(" + x + ", " + y + ")";
        }
    }

    /** @} */
}

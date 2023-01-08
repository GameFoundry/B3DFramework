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
    /// A two dimensional vector with integer coordinates.
    /// </summary>
    public partial struct Vector2I
    {
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
                        return X;
                    case 1:
                        return Y;
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector2I index.");
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
                    default:
                        throw new IndexOutOfRangeException("Invalid Vector2I index.");
                }
            }
        }

        /// <summary>
        /// Returns length of the vector.
        /// </summary>
        public float Length
        {
            get
            {
                return (float)MathEx.Sqrt(X * X + Y * Y);
            }
        }

        /// <summary>
        /// Returns the squared length of the vector.
        /// </summary>
        public float SqrdLength
        {
            get
            {
                return (X * X + Y * Y);
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
            return Math.Abs(b.X - a.X) + Math.Abs(b.Y - a.Y);
        }

        public static Vector2I operator +(Vector2I a, Vector2I b)
        {
            return new Vector2I(a.X + b.X, a.Y + b.Y);
        }

        public static Vector2I operator -(Vector2I a, Vector2I b)
        {
            return new Vector2I(a.X - b.X, a.Y - b.Y);
        }

        public static Vector2I operator -(Vector2I v)
        {
            return new Vector2I(-v.X, -v.Y);
        }

        public static Vector2I operator *(Vector2I v, int d)
        {
            return new Vector2I(v.X * d, v.Y * d);
        }

        public static Vector2I operator *(int d, Vector2I v)
        {
            return new Vector2I(v.X * d, v.Y * d);
        }

        public static Vector2I operator /(Vector2I v, int d)
        {
            return new Vector2I(v.X / d, v.Y / d);
        }

        public static bool operator ==(Vector2I lhs, Vector2I rhs)
        {
            return lhs.X == rhs.X && lhs.Y == rhs.Y;
        }

        public static bool operator !=(Vector2I lhs, Vector2I rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return X.GetHashCode() ^ Y.GetHashCode() << 2;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Vector2I))
                return false;

            Vector2I vec = (Vector2I)other;
            if (X.Equals(vec.X) && Y.Equals(vec.Y))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return "(" + X + ", " + Y + ")";
        }
    }

    /** @} */
}

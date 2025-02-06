//********************************* bs::framework - Copyright 2025 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
//global using Vector2F = bs.TVector2<float>;
//global using Vector2I = bs.TVector2<int>;

using System;
using System.Numerics;
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Math
     *  @{
     */

    public static class Vector2Extensions
    {
        /// <summary>
        /// Returns the length of the vector.
        /// </summary>
        public static T GetLength<T>(this TVector2<T> value) where T : IRootFunctions<T>, INumber<T>
        {
            return T.Sqrt(value.X * value.X + value.Y + value.Y);
        }

        /// <summary>
        /// Returns the length of the vector.
        /// </summary>
        public static float GetLength(this TVector2<int> value)
        {
            return float.Sqrt(value.X * value.X + value.Y + value.Y);
        }

        /// <summary>
        /// Returns the length of the vector.
        /// </summary>
        public static T GetLength<T, Unit>(this TVector2<TUnitValue<T, Unit>> value) where T : IRootFunctions<T>, INumber<T>
        {
            return T.Sqrt((T)(value.X * value.X + value.Y + value.Y));
        }
    }

    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct TVector2<T>(T x, T y)
        : IAdditionOperators<TVector2<T>, TVector2<T>, TVector2<T>>
        , ISubtractionOperators<TVector2<T>, TVector2<T>, TVector2<T>>
        , IMultiplyOperators<TVector2<T>, T, TVector2<T>>
        , IDivisionOperators<TVector2<T>, T, TVector2<T>>
        , IUnaryPlusOperators<TVector2<T>, TVector2<T>>
        , IUnaryNegationOperators<TVector2<T>, TVector2<T>>
        , IEqualityOperators<TVector2<T>, TVector2<T>, bool>
        where T : INumber<T>
    {
        public T X = x;
        public T Y = y;

        /// <summary>Initializes the struct with default values.</summary>
        public static TVector2<T> Default()
        {
            return new TVector2<T>();
        }

        public TVector2(T value) : this(value, value)
        {
        }

        /// <summary>
        /// Accesses a specific component of the vector.
        /// </summary>
        /// <param name = "index" > Index of the component.</param>
        /// <returns>Value of the specific component.</returns>
        public T this[int index]
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
                        throw new IndexOutOfRangeException("Invalid TVector2I index.");
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
                        throw new IndexOutOfRangeException("Invalid TVector2I index.");
                }
            }
        }

        /// <summary>
        /// Returns the squared length of the vector.
        /// </summary>
        public T SquaredLength => X * X + Y * Y;

        public static TVector2<T> operator +(TVector2<T> left, TVector2<T> right) => new (left.X + right.X, left.Y + right.Y);
        public static TVector2<T> operator -(TVector2<T> left, TVector2<T> right) => new (left.X - right.X, left.Y - right.Y);
        public static TVector2<T> operator *(TVector2<T> left, T right) => new (left.X * right, left.Y * right);
        public static TVector2<T> operator /(TVector2<T> left, T right) { return new TVector2<T>(left.X / right, left.Y / right); }
        public static TVector2<T> operator -(TVector2<T> value) => new (-value.X, -value.Y);
        public static TVector2<T> operator +(TVector2<T> value) => new (value.X, value.Y);

        public static bool operator ==(TVector2<T> lhs, TVector2<T> rhs) => lhs.X.Equals(rhs.X) && lhs.Y.Equals(rhs.Y);
        public static bool operator !=(TVector2<T> lhs, TVector2<T> rhs) => !(lhs == rhs);

        /// <summary>
        /// Returns the Manhattan distance between two points.
        /// </summary>
        public static int ManhattanDistance(TVector2<int> a, TVector2<int> b)
        {
            return int.Abs(b.X - a.X) + int.Abs(b.Y - a.Y);
        }

        public override int GetHashCode() => X.GetHashCode() ^ Y.GetHashCode() << 2;

        public override bool Equals(object other)
        {
            if (!(other is TVector2<T>))
                return false;

            TVector2<T> vector = (TVector2<T>)other;
            if (X.Equals(vector.X) && Y.Equals(vector.Y))
                return true;

            return false;
        }

        public override string ToString()
        {
            return "(" + X + ", " + Y + ")";
        }
    }

    /** @} */
}

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
        public static float GetLength<Unit>(this TVector2<TUnitValue<int, Unit>> value)
        {
            return float.Sqrt((float)value.X * (float)value.X + (float)value.Y + (float)value.Y);
        }

        /// <summary>
        /// Returns the length of the vector.
        /// </summary>
        public static T GetLength<T, Unit>(this TVector2<TUnitValue<T, Unit>> value) where T : IRootFunctions<T>, INumber<T>
        {
            return T.Sqrt((T)(value.X * value.X + value.Y + value.Y));
        }

        /// <summary>
        /// Normalizes the provided vector and returns the normalized copy.
        /// </summary>
        public static TVector2<T> Normalize<T>(this TVector2<T> value) where T : IFloatingPointIeee754<T>
        {
            T squaredLength = value.SquaredLength;
            if (squaredLength > T.CreateChecked(1e-04))
                return value * (T.One / T.Sqrt(squaredLength));

            return value;
        }

        /// <summary>
        /// Normalizes the provided vector and returns the normalized copy.
        /// </summary>
        public static TVector2<TUnitValue<T, Unit>> Normalize<T, Unit>(this TVector2<TUnitValue<T, Unit>> value) where T : IFloatingPointIeee754<T>
        {
            T squaredLength = (T)value.SquaredLength;
            if (squaredLength > T.CreateChecked(1e-04))
                return value * (T.One / T.Sqrt(squaredLength));

            return value;
        }
    }

    public partial struct TVector2<T>
        : IAdditionOperators<TVector2<T>, TVector2<T>, TVector2<T>>
        , ISubtractionOperators<TVector2<T>, TVector2<T>, TVector2<T>>
        , IMultiplyOperators<TVector2<T>, T, TVector2<T>>
        , IDivisionOperators<TVector2<T>, T, TVector2<T>>
        , IUnaryPlusOperators<TVector2<T>, TVector2<T>>
        , IUnaryNegationOperators<TVector2<T>, TVector2<T>>
        , IEqualityOperators<TVector2<T>, TVector2<T>, bool>
        where T : INumber<T>
    {
        public static readonly TVector2<T> kZero = new (T.CreateChecked(0), T.CreateChecked(0));
        public static readonly TVector2<T> kOne = new (T.CreateChecked(1), T.CreateChecked(1));
        public static readonly TVector2<T> kXAxis = new (T.CreateChecked(1), T.CreateChecked(0));
        public static readonly TVector2<T> kYAxis = new (T.CreateChecked(0), T.CreateChecked(1));

        /// <summary>Initializes the struct with default values.</summary>
        public static TVector2<T> Default()
        {
            return new TVector2<T>();
        }

        public TVector2(T x, T y)
        {
            X = x;
            Y = y;
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

        /// <summary>
        /// Scales the components of the vector by specified scale factors.
        /// </summary>
        /// <param name="scale">Scale factors to multiply components by.</param>
        public void Scale(TVector2<T> scale)
        {
            X *= scale.X;
            Y *= scale.Y;
        }

        /// <summary>
        /// Calculates the inner product of the two vectors.
        /// </summary>
        public static T Dot(TVector2<T> lhs, TVector2<T> rhs)
        {
            return lhs.X * rhs.X + lhs.Y * rhs.Y;
        }

        /// <summary>
        /// Calculates the cross product of the two vectors.
        /// </summary>
        public static T Cross(TVector2<T> lhs, TVector2<T> rhs)
        {
            return lhs.X * rhs.Y - lhs.Y * rhs.X;
        }

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
        public static T ManhattanDistance(TVector2<T> a, TVector2<T> b)
        {
            return T.Abs(b.X - a.X) + T.Abs(b.Y - a.Y);
        }

        /// <summary>
        /// Calculates the distance between two points.
        /// </summary>
        public static float Distance(TVector2<float> lhs, TVector2<float> rhs)
        {
            TVector2<float> difference = new TVector2<float>(lhs.X - rhs.X, lhs.Y - rhs.Y);
            return float.Sqrt(difference.X * difference.X + difference.Y * difference.Y);
        }

        /// <summary>
        /// Calculates the distance between two points.
        /// </summary>
        public static double Distance(TVector2<double> lhs, TVector2<double> rhs)
        {
            TVector2<double> difference = new TVector2<double>(lhs.X - rhs.X, lhs.Y - rhs.Y);
            return double.Sqrt(difference.X * difference.X + difference.Y * difference.Y);
        }

        /// <summary>
        /// Returns the maximum of all the vector components as a new vector.
        /// </summary>
        /// <returns>Vector consisting of maximum components of the first and second vector.</returns>
        public static TVector2<T> Max(TVector2<T> lhs, TVector2<T> rhs)
        {
            return new TVector2<T>(T.Max(lhs.X, rhs.X), T.Max(lhs.Y, rhs.Y));
        }

        /// <summary>
        /// Returns the minimum of all the vector components as a new vector.
        /// </summary>
        /// <returns>Vector consisting of minimum components of the first and second vector.</returns>
        public static TVector2<T> Min(TVector2<T> lhs, TVector2<T> rhs)
        {
            return new TVector2<T>(T.Min(lhs.X, rhs.X), T.Min(lhs.Y, rhs.Y));
        }

        /// <summary>
        /// Converts a vector with one underlying type to another.
        /// </summary>
        public TVector2<T2> To<T2>() where T2 : INumber<T2>
        {
            return new TVector2<T2>(T2.CreateChecked(X), T2.CreateChecked(Y));
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

//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Numerics;
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// A two dimensional size.
    /// </summary>
    public partial struct TSize2<T>
        : IAdditionOperators<TSize2<T>, TSize2<T>, TSize2<T>>
        , ISubtractionOperators<TSize2<T>, TSize2<T>, TSize2<T>>
        , IMultiplyOperators<TSize2<T>, T, TSize2<T>>
        , IDivisionOperators<TSize2<T>, T, TSize2<T>>
        , IUnaryPlusOperators<TSize2<T>, TSize2<T>>
        , IUnaryNegationOperators<TSize2<T>, TSize2<T>>
        , IEqualityOperators<TSize2<T>, TSize2<T>, bool>
        where T : INumber<T>
    {
        public static readonly TSize2<T> kZero = new (T.CreateChecked(0), T.CreateChecked(0));
        public static readonly TSize2<T> kOne = new (T.CreateChecked(1), T.CreateChecked(1));

        /// <summary>Initializes the struct with default values.</summary>
        public static TSize2<T> Default()
        {
            return new TSize2<T>();
        }

        public TSize2(T width, T height)
        {
            Width = width;
            Height = height;
        }

        public TSize2(T value) : this(value, value)
        {
        }

        public static TSize2<T> operator +(TSize2<T> left, TSize2<T> right) => new (left.Width + right.Width, left.Height + right.Height);
        public static TSize2<T> operator -(TSize2<T> left, TSize2<T> right) => new (left.Width - right.Width, left.Height - right.Height);

        public static TSize2<T> operator *(TSize2<T> left, T right) => new (left.Width * right, left.Height * right);
        public static TSize2<T> operator /(TSize2<T> left, T right) { return new TSize2<T>(left.Width / right, left.Height / right); }
        public static TSize2<T> operator -(TSize2<T> value) => new (-value.Width, -value.Height);
        public static TSize2<T> operator +(TSize2<T> value) => new (value.Width, value.Height);

        public static bool operator ==(TSize2<T> lhs, TSize2<T> rhs) => lhs.Width.Equals(rhs.Width) && lhs.Height.Equals(rhs.Height);
        public static bool operator !=(TSize2<T> lhs, TSize2<T> rhs) => !(lhs == rhs);

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return Width.GetHashCode() ^ Height.GetHashCode() << 2;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is TSize2<T>))
                return false;

            TSize2<T> size = (TSize2<T>)other;
            if (Width.Equals(size.Width) && Height.Equals(size.Height))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return "(" + Width + ", " + Height + ")";
        }
    }

    /** @} */
}

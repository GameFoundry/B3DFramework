//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Numerics;
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Math
     *  @{
     */

    public partial struct TArea2<PositionType, SizeType>
        : IEqualityOperators<TArea2<PositionType, SizeType>, TArea2<PositionType, SizeType>, bool>
        where PositionType : INumber<PositionType>
        where SizeType : INumber<SizeType>
    {
        /** Returns position of the area. */
        public TVector2<PositionType> Position => new TVector2<PositionType>(X, Y);

        /** Returns width/height of the area. */
        public TSize2<SizeType> Size => new TSize2<SizeType>(Width, Height);

        /// <summary>Initializes the struct with default values.</summary>
        public static TArea2<PositionType, SizeType> Default()
        {
            return new TArea2<PositionType, SizeType>();
        }

        /// <summary>
        /// Creates a new 2D area.
        /// </summary>
        /// <param name="x">Left-most coordinate of the area.</param>
        /// <param name="y">Top-most coordinate of the area.</param>
        /// <param name="width">Width of the area.</param>
        /// <param name="height">Height of the area.</param>
        public TArea2(PositionType x, PositionType y, SizeType width, SizeType height)
        {
            X = x;
            Y = y;
            Width = width;
            Height = height;
        }

        /// <summary>
        /// Creates a new 2D area.
        /// </summary>
        /// <param name="position">Position of the top-left corner of the area.</param>
        /// <param name="size">Width/height of the area.</param>
        public TArea2(TVector2<PositionType> position, TSize2<SizeType> size)
        {
            X = position.X;
            Y = position.Y;
            Width = size.Width;
            Height = size.Height;
        }

        public static bool operator== (TArea2<PositionType, SizeType> lhs, TArea2<PositionType, SizeType> rhs)
        {
            return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Width == rhs.Width && lhs.Height == rhs.Height;
        }

        public static bool operator!= (TArea2<PositionType, SizeType> lhs, TArea2<PositionType, SizeType> rhs)
        {
            return !(lhs == rhs);
        }

        /// <summary>
        /// Returns true if the area contains the provided point.
        /// </summary>
        /// <param name="point">Point to check if it is in the area.</param>
        /// <returns>True if the point within the area.</returns>
        public bool Contains(TVector2<PositionType> point)
        {
            if(point.X >= X && point.X < (X + PositionType.CreateTruncating(Width)))
            {
                if(point.Y >= Y && point.Y < (Y + PositionType.CreateTruncating(Height)))
                    return true;
            }

            return false;
        }

        /// <summary>
        /// Returns true if the area overlaps the provided area. Also returns true if the areas are
        /// contained within each other completely (no intersecting edges).
        /// </summary>
        /// <param name="other">Other area to compare with.</param>
        /// <returns>True if the areas overlap.</returns>
        public bool Overlaps(TArea2<PositionType, SizeType> other)
        {
            PositionType otherRight = other.X + PositionType.CreateTruncating(other.Width);
            PositionType myRight = X + PositionType.CreateTruncating(Width);

            PositionType otherBottom = other.Y + PositionType.CreateTruncating(other.Height);
            PositionType myBottom = Y + PositionType.CreateTruncating(Height);

            if(X < otherRight && myRight > other.X && Y < otherBottom && myBottom > other.Y)
                return true;

            return false;
        }

        /// <summary>
        /// Clips current area so that it does not overlap the provided area. After clipping no area of this
        /// area will intersect the clip area.
        /// </summary>
        /// <param name="clipArea">Area to clip against.</param>
        public void Clip(TArea2<PositionType, SizeType> clipArea)
        {
            PositionType newLeft = PositionType.Max(X, clipArea.X);
            PositionType newTop = PositionType.Max(Y, clipArea.Y);

            PositionType newRight = PositionType.Min(X + PositionType.CreateTruncating(Width), clipArea.X + PositionType.CreateTruncating(clipArea.Width));
            PositionType newBottom = PositionType.Min(Y + PositionType.CreateTruncating(Height), clipArea.Y + PositionType.CreateTruncating(clipArea.Height));

            X = PositionType.Min(newLeft, newRight);
            Y = PositionType.Min(newTop, newBottom);
            Width = SizeType.CreateTruncating(PositionType.Max(PositionType.Zero, newRight - newLeft));
            Height = SizeType.CreateTruncating(PositionType.Max(PositionType.Zero, newBottom - newTop));
        }

        /// <summary>
        /// Converts an area with one underlying type to another.
        /// </summary>
        public TArea2<PositionType2, SizeType2> To<PositionType2, SizeType2>()
            where PositionType2 : INumber<PositionType2>
            where SizeType2 : INumber<SizeType2>
        {
            return new TArea2<PositionType2, SizeType2>(PositionType2.CreateTruncating(X), PositionType2.CreateTruncating(Y), SizeType2.CreateTruncating(Width), SizeType2.CreateTruncating(Height));
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is TArea2<PositionType, SizeType>))
                return false;

            TArea2<PositionType, SizeType> otherArea = (TArea2<PositionType, SizeType>)other;
            if (X.Equals(otherArea.X) && Y.Equals(otherArea.Y) && Width.Equals(otherArea.Width) && Height.Equals(otherArea.Height))
                return true;

            return false;
        }

        public override int GetHashCode()
        {
            return X.GetHashCode() ^ Y.GetHashCode() ^ Width.GetHashCode() ^ Height.GetHashCode() << 2;
        }

        public override string ToString()
        {
            return String.Format("(X:{0} Y:{1} Width:{2} Height:{3})", X, Y, Width, Height);
        }
    }

    /** @} */
}

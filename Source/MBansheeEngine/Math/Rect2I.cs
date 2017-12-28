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
    /// Represents a 2D rectangle using integer values. Rectangle is represented with an origin in top left and 
    /// width/height.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Rect2I
    {
        public int x, y, width, height;

        /// <summary>
        /// Creates a new 2D rectangle.
        /// </summary>
        /// <param name="x">Left-most coordinate of the rectangle.</param>
        /// <param name="y">Top-most coordinate of the rectangle.</param>
        /// <param name="width">Width of the rectangle.</param>
        /// <param name="height">Height of the rectangle.</param>
        public Rect2I(int x, int y, int width, int height)
        {
            this.x = x;
            this.y = y;
            this.width = width;
            this.height = height;
        }

        public static bool operator== (Rect2I lhs, Rect2I rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.width == rhs.width && lhs.height == rhs.height;
        }

        public static bool operator!= (Rect2I lhs, Rect2I rhs)
        {
            return !(lhs == rhs);
        }

        /// <summary>
        /// Returns true if the rectangle contains the provided point.
        /// </summary>
        /// <param name="point">Point to check if it is in rectangle.</param>
        /// <returns>True if the point within rectangle bounds.</returns>
        public bool Contains(Vector2I point)
        {
            if(point.x >= x && point.x < (x + width))
            {
                if(point.y >= y && point.y < (y + height))
                    return true;
            }

            return false;
        }

        /// <summary>
        /// Returns true if the rectangle overlaps the provided rectangle. Also returns true if the rectangles are 
        // contained within each other completely (no intersecting edges).
        /// </summary>
        /// <param name="other">Other rectangle to compare with.</param>
        /// <returns>True if the rectangles overlap.</returns>
        public bool Overlaps(Rect2I other)
        {
            int otherRight = other.x + other.width;
            int myRight = x + width;

            int otherBottom = other.y + other.height;
            int myBottom = y + height;

            if(x < otherRight && myRight > other.x &&
                y < otherBottom && myBottom > other.y)
                return true;

            return false;
        }

        /// <summary>
        /// Clips current rectangle so that it does not overlap the provided rectangle. After clipping no area of this
        /// rectangle will intersect the clip area.
        /// </summary>
        /// <param name="clipRect">Rectangle to clip against.</param>
        public void Clip(Rect2I clipRect)
        {
            int newLeft = Math.Max(x, clipRect.x);
            int newTop = Math.Max(y, clipRect.y);

            int newRight = Math.Min(x + width, clipRect.x + clipRect.width);
            int newBottom = Math.Min(y + height, clipRect.y + clipRect.height);

            x = Math.Min(newLeft, newRight);
            y = Math.Min(newTop, newBottom);
            width = Math.Max(0, newRight - newLeft);
            height = Math.Max(0, newBottom - newTop);
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Rect2I))
                return false;

            Rect2I rect = (Rect2I)other;
            if (x.Equals(rect.x) && y.Equals(rect.y) && width.Equals(rect.width) && height.Equals(rect.height))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return String.Format("(x:{0} y:{1} width:{2} height:{3})", x, y, width, height);
        }
    }

    /** @} */
}

//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
    /// Represents a 2D rectangle using real values. Rectangle is represented with an origin in top left and width/height.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Rect2 // Note: Must match C++ struct Rect2
    {
        public float x, y, width, height;

        /// <summary>
        /// Creates a new 2D rectangle.
        /// </summary>
        /// <param name="x">Left-most coordinate of the rectangle.</param>
        /// <param name="y">Top-most coordinate of the rectangle.</param>
        /// <param name="width">Width of the rectangle.</param>
        /// <param name="height">Height of the rectangle.</param>
        public Rect2(float x, float y, float width, float height)
        {
            this.x = x;
            this.y = y;
            this.width = width;
            this.height = height;
        }

        public static bool operator ==(Rect2 lhs, Rect2 rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.width == rhs.width && lhs.height == rhs.height;
        }

        public static bool operator !=(Rect2 lhs, Rect2 rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Rect2))
                return false;

            Rect2 rect = (Rect2)other;
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

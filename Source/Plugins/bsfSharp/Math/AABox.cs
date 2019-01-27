//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// Axis aligned box represented by minimum and maximum point.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct AABox // Note: Must match C++ class AABox
    {
        [SerializeField]
        private Vector3 minimum;

        [SerializeField]
        private Vector3 maximum;

        /// <summary>
        /// Corner of the box with minimum values (opposite to maximum corner).
        /// </summary>
        public Vector3 Minimum
        {
            get { return minimum; }
            set { minimum = value; }
        }

        /// <summary>
        /// Corner of the box with maximum values (opposite to minimum corner).
        /// </summary>
        public Vector3 Maximum
        {
            get { return maximum; }
            set { maximum = value; }
        }

        /// <summary>
        /// Returns the center of the box.
        /// </summary>
        public Vector3 Center
        {
            get 
            { 		
                return new Vector3((maximum.x + minimum.x) * 0.5f,
                        (maximum.y + minimum.y) * 0.5f,
                        (maximum.z + minimum.z) * 0.5f);
            }
        }

        /// <summary>
        /// Returns the width, height and depth of the box.
        /// </summary>
        public Vector3 Size
        {
            get
            {
                return maximum - minimum;
            }
        }

        /// <summary>
        /// Creates a new axis aligned box.
        /// </summary>
        /// <param name="min">Corner of the box with minimum values.</param>
        /// <param name="max">Corner of the box with maximum values.</param>
        public AABox(Vector3 min, Vector3 max)
        {
            minimum = min;
            maximum = max;
        }

        /// <summary>
        /// Transforms the bounding box by the given matrix.
        /// 
        /// As the resulting box will no longer be axis aligned, an axis align box is instead created by encompassing the 
        /// transformed oriented bounding box. Retrieving the value as an actual OBB would provide a tighter fit.
        /// </summary>
        /// <param name="tfrm">Affine matrix to transform the box with.</param>
        public void TransformAffine(Matrix4 tfrm)
        {
            Vector3 center = Center;
            Vector3 halfSize = Size*0.5f;

            Vector3 newCenter = tfrm.MultiplyAffine(center);
            Vector3 newHalfSize = new Vector3(
                MathEx.Abs(tfrm.m00) * halfSize.x + MathEx.Abs(tfrm.m01) * halfSize.y + MathEx.Abs(tfrm.m02) * halfSize.z,
                MathEx.Abs(tfrm.m10) * halfSize.x + MathEx.Abs(tfrm.m11) * halfSize.y + MathEx.Abs(tfrm.m12) * halfSize.z,
                MathEx.Abs(tfrm.m20) * halfSize.x + MathEx.Abs(tfrm.m21) * halfSize.y + MathEx.Abs(tfrm.m22) * halfSize.z);

            minimum = newCenter - newHalfSize;
            maximum = newCenter + newHalfSize;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return "Min: " + minimum + ". Max: " + maximum;
        }
    };

    /** @} */
}

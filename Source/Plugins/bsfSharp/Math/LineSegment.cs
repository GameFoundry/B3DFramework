//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
    /// Represents a line segment in three dimensional space defined by a start and an end point.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct LineSegment
    {
        [SerializeField]
        private Vector3 start;

        [SerializeField]
        private Vector3 end;

        /// <summary>
        /// Start position of the line segment.
        /// </summary>
        public Vector3 Start
        {
            get { return start; }
            set { start = value; }
        }

        /// <summary>
        /// End position of the line segment.
        /// </summary>
        public Vector3 End
        {
            get { return end; }
            set { end = value; }
        }

        /// <summary>
        /// Creates a new line segment.
        /// </summary>
        /// <param name="start">Line segment start position.</param>
        /// <param name="end">Line segment end position.</param>
        public LineSegment(Vector3 start, Vector3 end)
        {
            this.start = start;
            this.end = end;
        }
    }

    /** @} */
}

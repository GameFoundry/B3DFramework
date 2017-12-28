//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// Represents a rectangle in three dimensional space. It is represented by two axes that extend from the specified 
    /// origin. Axes should be perpendicular to each other and they extend in both positive and negative directions from 
    /// the origin by the amount specified by extents.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Rect3 // Note: Must match C++ class Rect3
    {
        /// <summary>
        /// Creates a new rectangle.
        /// </summary>
        /// <param name="center">Origin of the rectangle. </param>
        /// <param name="axes">Two axes that define orientation of the rectangle. Axes extend from the origin. Axes should 
        ///                    be normalized.</param>
        /// <param name="extents">Two extents that define the size of the rectangle. Extends should be half the width/height
        ///                       as they are applied in both directions.</param>
        public Rect3(Vector3 center, Vector3[] axes, float[] extents)
        {
            this._center = center;
            _axisHorz = axes[0];
            _axisVert = axes[1];
            _extentHorz = extents[0];
            _extentVert = extents[1];
        }

        /// <summary>
        /// Origin of the rectangle.
        /// </summary>
        public Vector3 Center
        {
            get { return _center; }
            set { _center = value; }
        }

        /// <summary>
        /// Returns the horizontal axis. Together with the vertical axis this defines rectangle orientation.
        /// </summary>
        public Vector3 AxisHorz
        {
            get { return _axisHorz; }
            set { _axisHorz = value; }
        }

        /// <summary>
        /// Returns the vertical axis. Together with the horizontal axis this defines rectangle orientation.
        /// </summary>
        public Vector3 AxisVert
        {
            get { return _axisVert; }
            set { _axisVert = value; }
        }

        /// <summary>
        /// Returns the extents in the direction of the horizontal axis. This represents half the total
        /// width of the rectangle.
        /// </summary>
        public float ExtentHorz
        {
            get { return _extentHorz; }
            set { _extentHorz = value; }
        }

        /// <summary>
        /// Returns the extents in the direction of the vertical axis. This represents half the total
        /// height of the rectangle.
        /// </summary>
        public float ExtentVert
        {
            get { return _extentVert; }
            set { _extentVert = value; }
        }

        [SerializeField]
        private Vector3 _center;
        [SerializeField]
        private Vector3 _axisHorz;
        [SerializeField]
        private Vector3 _axisVert;
        [SerializeField]
        private float _extentHorz;
        [SerializeField]
        private float _extentVert;
    };

    /** @} */
}

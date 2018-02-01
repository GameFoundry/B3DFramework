//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Handles
     *  @{
     */

    /// <summary>
    /// Constraint that determines in which direction can a HandleSlider2D be moved.
    /// </summary>
    public enum Slider2DConstraint // Note: Must match C++ enum Slider2DConstraint
    {
        /// <summary>
        /// Slider can be moved in both dimensions.
        /// </summary>
        None,

        
        /// <summary>
        /// Slider can only be moved horizontally. 
        /// </summary>
        Horizontal,

        /// <summary>
        /// Slider can only be moved vertically.
        /// </summary>
        Vertical
    };

    /// <summary>
    /// Handle slider that is positioned in screen-space, and reports 2D movement in screen space (in pixels). When setting
    /// the position the Z coordinate will be ignored, and XY coordinates will be interpreted as pixels relative to the
    /// camera its viewed through.
    /// </summary>
    public sealed class HandleSlider2D : HandleSlider
    {
        /// <summary>
        /// Creates a new 2D handle slider. 
        /// </summary>
        /// <param name="parentHandle">Handle that the slider belongs to.</param>
        /// <param name="width">Width of the area of the slider that can be interacted with, in pixels.</param>
        /// <param name="height">Height of the area of the slider that can be interacted with, in pixels.</param>
        /// <param name="layer">
        /// Layer that allows filtering of which sliders are interacted with from a specific camera.
        /// </param>
        /// <param name="constraint">
        /// Optional constraint that determines in which direction is the slider allowed to be moved in.
        /// </param>
        public HandleSlider2D(Handle parentHandle, int width, int height, Slider2DConstraint constraint, UInt64 layer = 1)
            :base(parentHandle)
        {
            Internal_CreateInstance(this, width, height, constraint, layer);
        }

        /// <summary>
        /// Returns a delta value that is the result of dragging/sliding the pointer. This changes every frame and will be
        /// zero unless the slider is active. The value is in screen space (pixels).
        /// </summary>
        public Vector2I Delta
        {
            get
            {
                Vector2I value;
                Internal_GetDelta(mCachedPtr, out value);
                return value;
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(HandleSlider2D instance, int width, int height, 
            Slider2DConstraint constraint, UInt64 layer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetDelta(IntPtr nativeInstance, out Vector2I value);
    }

    /** @} */
}

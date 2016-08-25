//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */

    /// <summary>
    /// Base class that can be implemented by objects needing to elements along draw a horizontal timeline.
    /// </summary>
    public class GUITimelineBase
    {
        public const int PADDING = 30;

        protected int drawableWidth;
        protected float rangeLength = 60.0f;
        protected float rangeOffset = 0.0f;

        protected GUICanvas canvas;
        protected int width;
        protected int height;
        protected int fps = 1;
        protected int markedFrameIdx = -1;

        /// <summary>
        /// Constructs a new timeline and adds it to the specified layout.
        /// </summary>
        /// <param name="layout">Layout to add the timeline GUI to.</param>
        /// <param name="width">Width of the timeline in pixels.</param>
        /// <param name="height">Height of the timeline in pixels.</param>
        public GUITimelineBase(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            SetSize(width, height);
        }

        /// <summary>
        /// Uses the assigned FPS, range and physical size to calculate the frame that is under the provided coordinates.
        /// </summary>
        /// <param name="pixelCoords">Coordinate relative to the layout the GUI element is on.</param>
        /// <returns>Frame that was clicked on, or -1 if the coordinates are outside of valid bounds. </returns>
        public int GetFrame(Vector2I pixelCoords)
        {
            Rect2I bounds = canvas.Bounds;

            if (pixelCoords.x < (bounds.x + PADDING) || pixelCoords.x >= (bounds.x + bounds.width - PADDING) ||
                pixelCoords.y < bounds.y || pixelCoords.y >= (bounds.y + bounds.height))
            {
                return -1;
            }

            Vector2I relativeCoords = pixelCoords - new Vector2I(bounds.x + PADDING, bounds.y);

            float lengthPerPixel = GetRange() / drawableWidth;
            float time = rangeOffset + relativeCoords.x * lengthPerPixel;

            return MathEx.RoundToInt(time * fps);
        }

        /// <summary>
        /// Finds the pixel offset relative to the GUI element's origin, of the specified time.
        /// </summary>
        /// <param name="time">Time value to return the offset for.</param>
        /// <returns>Offset in pixels relative to GUI element's origin.</returns>
        public int GetOffset(float time)
        {
            return (int)(((time - rangeOffset) / GetRange()) * drawableWidth) + PADDING;
        }

        /// <summary>
        /// Returns time for a frame with the specified index. Depends on set range and FPS.
        /// </summary>
        /// <param name="frameIdx">Index of the frame (not a key-frame) to get the time for.</param>
        /// <returns>Time of the frame with the provided index. </returns>
        public float GetTimeForFrame(int frameIdx)
        {
            float range = GetRange();
            int numFrames = (int)range * fps;
            float timePerFrame = range / numFrames;

            return frameIdx * timePerFrame;
        }

        /// <summary>
        /// Sets the frame at which to display the frame marker.
        /// </summary>
        /// <param name="frameIdx">Index of the frame to display the marker on, or -1 to clear the marker.</param>
        public void SetMarkedFrame(int frameIdx)
        {
            markedFrameIdx = frameIdx;
        }

        /// <summary>
        /// Sets the physical size onto which to draw the timeline.
        /// </summary>
        /// <param name="width">Width in pixels.</param>
        /// <param name="height">Height in pixels.</param>
        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            drawableWidth = Math.Max(0, width - PADDING * 2);
        }

        /// <summary>
        /// Sets the range of values to display on the timeline.
        /// </summary>
        /// <param name="length">Amount of time to display, in seconds.</param>
        public void SetRange(float length)
        {
            rangeLength = Math.Max(0.0f, length);
        }

        /// <summary>
        /// Returns the offset at which the displayed timeline values start at.
        /// </summary>
        /// <param name="offset">Value to start the timeline values at, in seconds.</param>
        public void SetOffset(float offset)
        {
            rangeOffset = offset;
        }

        /// <summary>
        /// Number of frames per second, used for frame selection and marking.
        /// </summary>
        /// <param name="fps">Number of prames per second.</param>
        public void SetFPS(int fps)
        {
            this.fps = Math.Max(1, fps);
        }

        /// <summary>
        /// Draws a vertical frame marker at the specified time.
        /// </summary>
        /// <param name="t">Time at which to draw the marker.</param>
        private void DrawFrameMarker(float t)
        {
            int xPos = (int)(((t - rangeOffset) / GetRange()) * drawableWidth) + PADDING;

            Vector2I start = new Vector2I(xPos, 0);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, Color.BansheeOrange);
        }

        /// <summary>
        /// Returns the range of times displayed by the timeline rounded to the multiple of FPS.
        /// </summary>
        /// <param name="padding">If true, extra range will be included to cover the right-most padding.</param>
        /// <returns>Time range rounded to a multiple of FPS.</returns>
        protected float GetRange(bool padding = false)
        {
            float spf = 1.0f / fps;

            float range = rangeLength;
            if (padding)
            {
                float lengthPerPixel = rangeLength / drawableWidth;
                range += lengthPerPixel * PADDING;
            }

            return MathEx.Max(1.0f, range / spf) * spf;
        }

        /// <summary>
        /// Draws the frame marker at the currently selected frame.
        /// </summary>
        protected void DrawFrameMarker()
        {
            if (markedFrameIdx != -1)
            {
                float range = GetRange();
                int numFrames = (int)range * fps;
                float timePerFrame = range / numFrames;

                DrawFrameMarker(markedFrameIdx * timePerFrame);
            }
        }

        /// <summary>
        /// Rebuilds the internal GUI elements. Should be called whenever timeline properties change.
        /// </summary>
        public virtual void Rebuild()
        {
            canvas.Clear();
        }
    }

    /** @} */
}

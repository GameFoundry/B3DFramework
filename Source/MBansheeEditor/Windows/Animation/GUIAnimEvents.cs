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
    /// Renders a list of animation events in a form of a timeline. User can set the range of the times to display,
    /// as well as its physical dimensions.
    /// </summary>
    public class GUIAnimEvents
    {
        private const int EVENT_HALF_WIDTH = 2;

        private float rangeLength = 60.0f;
        private float rangeOffset = 0.0f;
        private int fps = 1;

        private GUICanvas canvas;
        private int width;
        private int height;
        private int drawableWidth;

        private AnimationEvent[] events = new AnimationEvent[0];
        private bool[] selectedEvents = new bool[0];

        /// <summary>
        /// Constructs a new events timeline and adds it to the specified layout.
        /// </summary>
        /// <param name="layout">Layout to add the events GUI to.</param>
        /// <param name="width">Width of the GUI element in pixels.</param>
        /// <param name="height">Height of the GUI element in pixels.</param>
        public GUIAnimEvents(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            SetSize(width, height);
        }

        /// <summary>
        /// Attempts to find an event under the provided coordinates.
        /// </summary>
        /// <param name="pixelCoords">Coordinates relative to the layout the GUI element is on.</param>
        /// <param name="eventIdx">Index of the event that was clicked on. Index references the events array as provided
        ///                        to <see cref="SetEvents"/>. Only valid if method returns true.</param>
        /// <returns>True if an event was found under the coordinates, false otherwise.</returns>
        public bool FindEvent(Vector2I pixelCoords, out int eventIdx)
        {
            Rect2I bounds = canvas.Bounds;

            if (pixelCoords.x < (bounds.x + GUIGraphTime.PADDING) || pixelCoords.x >= (bounds.x + bounds.width - GUIGraphTime.PADDING) ||
                pixelCoords.y < bounds.y || pixelCoords.y >= (bounds.y + bounds.height))
            {
                eventIdx = -1;
                return false;
            }

            Vector2I relativeCoords = pixelCoords - new Vector2I(bounds.x + GUIGraphTime.PADDING, bounds.y);
            for (int i = 0; i < events.Length; i++)
            {
                AnimationEvent evnt = events[i];

                int xPos = (int)(((evnt.Time - rangeOffset) / GetRange()) * drawableWidth) + GUIGraphTime.PADDING;

                if (relativeCoords.x >= (xPos - EVENT_HALF_WIDTH) || relativeCoords.y >= (xPos + EVENT_HALF_WIDTH))
                {
                    eventIdx = i;
                    return true;
                }
            }

            eventIdx = -1;
            return false;
        }

        /// <summary>
        /// Sets the physical size of the GUI element.
        /// </summary>
        /// <param name="width">Width in pixels.</param>
        /// <param name="height">Height in pixels.</param>
        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            drawableWidth = Math.Max(0, width - GUIGraphTime.PADDING * 2);
        }

        /// <summary>
        /// Sets the range of values over which to display the events.
        /// </summary>
        /// <param name="length">Amount of time to display, in seconds.</param>
        public void SetRange(float length)
        {
            rangeLength = Math.Max(0.0f, length);
        }

        /// <summary>
        /// Returns the offset at which the displayed event values start at.
        /// </summary>
        /// <param name="offset">Value to start displaying the events at, in seconds.</param>
        public void SetOffset(float offset)
        {
            rangeOffset = offset;
        }

        /// <summary>
        /// Number of frames per second, used for rounding up the displayed range.
        /// </summary>
        /// <param name="fps">Number of prames per second.</param>
        public void SetFPS(int fps)
        {
            this.fps = Math.Max(1, fps);
        }

        /// <summary>
        /// Changes the set of displayed animation events.
        /// </summary>
        /// <param name="events">Events to display on the timeline.</param>
        /// <param name="selected">Array of the same size as the <paramref name="events"/> array, determining which
        ///                        events should be displayed as selected.</param>
        public void SetEvents(AnimationEvent[] events, bool[] selected)
        {
            int numEvents;
            if (events != null)
                numEvents = events.Length;
            else
                numEvents = 0;

            this.events = new AnimationEvent[numEvents];
            if(events != null)
                Array.Copy(events, this.events, numEvents);

            selectedEvents = new bool[numEvents];
            if(selected != null)
                Array.Copy(selected, selectedEvents, MathEx.Min(numEvents, selected.Length));
        }

        /// <summary>
        /// Draws a marker for a single event.
        /// </summary>
        /// <param name="t">Time to draw the marker at.</param>
        /// <param name="selected">If true the marker will be drawn as selected.</param>
        private void DrawEventMarker(float t, bool selected)
        {
            int xPos = (int)(((t - rangeOffset) / GetRange()) * drawableWidth) + GUIGraphTime.PADDING;

            Vector2I a = new Vector2I(xPos - EVENT_HALF_WIDTH, height - 1);
            Vector2I b = new Vector2I(xPos, 0);
            Vector2I c = new Vector2I(xPos + EVENT_HALF_WIDTH, height - 1);
            Vector2I d = new Vector2I(xPos, 0);

            // Draw square shape
            Vector2I[] linePoints = { a, b, c, d, a };
            Vector2I[] trianglePoints = { b, c, a, d };

            Color outerColor = selected ? Color.BansheeOrange : Color.Black;
            canvas.DrawTriangleStrip(trianglePoints, Color.White, 101);
            canvas.DrawPolyLine(linePoints, outerColor, 100);
        }

        /// <summary>
        /// Returns the range of times displayed by the timeline rounded to the multiple of FPS.
        /// </summary>
        /// <param name="padding">If true, extra range will be included to cover the right-most padding.</param>
        /// <returns>Time range rounded to a multiple of FPS.</returns>
        private float GetRange(bool padding = false)
        {
            float spf = 1.0f / fps;

            float range = rangeLength;
            if (padding)
            {
                float lengthPerPixel = rangeLength / drawableWidth;
                range += lengthPerPixel * GUIGraphTime.PADDING;
            }

            return ((int)range / spf) * spf;
        }

        /// <summary>
        /// Rebuilds the internal GUI elements. Should be called whenever timeline properties change.
        /// </summary>
        public void Rebuild()
        {
            canvas.Clear();

            float range = GetRange();

            float lengthPerPixel = rangeLength / drawableWidth;
            float eventHalfWidth = lengthPerPixel * EVENT_HALF_WIDTH;
            for (int i = 0; i < events.Length; i++)
            {
                float t = events[i].Time;

                float min = t - eventHalfWidth;
                float max = t + eventHalfWidth;

                if (max < rangeOffset || min > (rangeOffset + range))
                    continue;

                DrawEventMarker(t, selectedEvents[i]);
            }
        }
    }

    /** @} */
}
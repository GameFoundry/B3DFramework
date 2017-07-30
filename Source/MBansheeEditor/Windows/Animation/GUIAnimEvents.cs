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
    public class GUIAnimEvents : GUITimelineBase
    {
        private const int EVENT_HALF_WIDTH = 2;

        private AnimationEvent[] events = new AnimationEvent[0];
        private bool[] selectedEvents = new bool[0];

        /// <summary>
        /// Constructs a new events timeline and adds it to the specified layout.
        /// </summary>
        /// <param name="layout">Layout to add the events GUI to.</param>
        /// <param name="width">Width of the GUI element in pixels.</param>
        /// <param name="height">Height of the GUI element in pixels.</param>
        public GUIAnimEvents(GUILayout layout, int width, int height)
            :base(layout, width, height)
        { }

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

            if (pixelCoords.x < (bounds.x + PADDING) || pixelCoords.x >= (bounds.x + bounds.width - PADDING) ||
                pixelCoords.y < bounds.y || pixelCoords.y >= (bounds.y + bounds.height))
            {
                eventIdx = -1;
                return false;
            }

            Vector2I relativeCoords = pixelCoords - new Vector2I(bounds.x, bounds.y);
            for (int i = 0; i < events.Length; i++)
            {
                AnimationEvent evnt = events[i];

                int xPos = (int)(((evnt.time - rangeOffset) / GetRange()) * drawableWidth) + PADDING;
                if (relativeCoords.x >= (xPos - EVENT_HALF_WIDTH) && relativeCoords.x <= (xPos + EVENT_HALF_WIDTH))
                {
                    eventIdx = i;
                    return true;
                }
            }

            eventIdx = -1;
            return false;
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
            int xPos = (int)(((t - rangeOffset) / GetRange()) * drawableWidth) + PADDING;

            Vector2I a = new Vector2I(xPos - EVENT_HALF_WIDTH, 0);
            Vector2I b = new Vector2I(xPos + EVENT_HALF_WIDTH, 0);
            Vector2I c = new Vector2I(xPos + EVENT_HALF_WIDTH, height - 1);
            Vector2I d = new Vector2I(xPos - EVENT_HALF_WIDTH, height - 1);

            // Draw square shape
            Vector2I[] linePoints = { a, b, c, d, a };
            Vector2I[] trianglePoints = { b, c, a, d };

            Color outerColor = selected ? Color.BansheeOrange : Color.Black;
            canvas.DrawTriangleStrip(trianglePoints, Color.White, 101);
            canvas.DrawPolyLine(linePoints, outerColor, 100);
        }
        
        /// <inheritdoc/>
        public override void Rebuild()
        {
            canvas.Clear();

            float range = GetRange();

            float lengthPerPixel = rangeLength / drawableWidth;
            float eventHalfWidth = lengthPerPixel * EVENT_HALF_WIDTH;
            for (int i = 0; i < events.Length; i++)
            {
                float t = events[i].time;

                float min = t - eventHalfWidth;
                float max = t + eventHalfWidth;

                if (max < rangeOffset || min > (rangeOffset + range))
                    continue;

                DrawEventMarker(t, selectedEvents[i]);
            }

            DrawFrameMarker();
        }
    }

    /** @} */
}
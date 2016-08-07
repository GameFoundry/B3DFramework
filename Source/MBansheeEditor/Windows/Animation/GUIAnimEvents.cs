//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */

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

        public GUIAnimEvents(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            SetSize(width, height);
        }

        public bool FindEvent(Vector2I windowCoords, out int eventIdx)
        {
            Rect2I bounds = canvas.Bounds;

            if (windowCoords.x < (bounds.x + GUIGraphTime.PADDING) || windowCoords.x >= (bounds.x + bounds.width - GUIGraphTime.PADDING) ||
                windowCoords.y < bounds.y || windowCoords.y >= (bounds.y + bounds.height))
            {
                eventIdx = -1;
                return false;
            }

            Vector2I relativeCoords = windowCoords - new Vector2I(bounds.x + GUIGraphTime.PADDING, bounds.y);
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

        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            drawableWidth = Math.Max(0, width - GUIGraphTime.PADDING * 2);
        }

        public void SetRange(float length)
        {
            rangeLength = Math.Max(0.0f, length);
        }

        public void SetOffset(float offset)
        {
            rangeOffset = offset;
        }

        public void SetFPS(int fps)
        {
            this.fps = Math.Max(1, fps);
        }

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
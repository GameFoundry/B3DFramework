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
    /// Renders a timeline that may be used as a header for a graph display. User can set the range of the times to display,
    /// as well as its physical dimensions.
    /// </summary>
    public class GUIGraphTime
    {
        public const int PADDING = 30;

        private const float TICK_HEIGHT_PCT = 0.4f;
        private const int TEXT_PADDING = 2;
        
        private int tickHeight;
        private int drawableWidth;
        private float rangeLength = 60.0f;

        private GUICanvas canvas;
        private GUIGraphTicks tickHandler;
        private int width;
        private int height;
        private int fps = 1;
        private int markedFrameIdx = -1;

        /// <summary>
        /// Constructs a new timeline and adds it to the specified layout.
        /// </summary>
        /// <param name="layout">Layout to add the timeline GUI to.</param>
        /// <param name="width">Width of the timeline in pixels.</param>
        /// <param name="height">Height of the timeline in pixels.</param>
        public GUIGraphTime(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            tickHandler = new GUIGraphTicks(GUITickStepType.Time);

            SetSize(width, height);
        }

        /// <summary>
        /// Uses the assigned FPS, range and physical size to calculate the frame that is under the provided coordinates.
        /// </summary>
        /// <param name="windowCoords">Coordinate relative to the window the GUI element is on.</param>
        /// <returns>Frame that was clicked on, or -1 if the coordinates are outside of valid bounds. </returns>
        public int GetFrame(Vector2I windowCoords)
        {
            Rect2I bounds = canvas.Bounds;

            if (windowCoords.x < (bounds.x + PADDING) || windowCoords.x >= (bounds.x + bounds.width - PADDING) ||
                windowCoords.y < bounds.y || windowCoords.y >= (bounds.y + bounds.height))
            {
                return -1;
            }

            Vector2I relativeCoords = windowCoords - new Vector2I(bounds.x + PADDING, bounds.y);

            float lengthPerPixel = GetRange() / drawableWidth;
            float time = relativeCoords.x * lengthPerPixel;

            return MathEx.RoundToInt(time * fps);
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

            tickHeight = (int)(height * TICK_HEIGHT_PCT);
            drawableWidth = Math.Max(0, width - PADDING * 2);

            tickHandler.SetRange(0.0f, GetRange(true), drawableWidth + PADDING);
        }

        /// <summary>
        /// Sets the range of values to display on the timeline.
        /// </summary>
        /// <param name="length">Amount of time to display, in seconds.</param>
        public void SetRange(float length)
        {
            rangeLength = Math.Max(0.0f, length);

            tickHandler.SetRange(0.0f, GetRange(true), drawableWidth + PADDING);
        }

        /// <summary>
        /// Number of frames per second, used for frame selection and marking.
        /// </summary>
        /// <param name="fps">Number of prames per second.</param>
        public void SetFPS(int fps)
        {
            this.fps = Math.Max(1, fps);

            tickHandler.SetRange(0.0f, GetRange(true), drawableWidth + PADDING);
        }
        
        /// <summary>
        /// Draws text displaying the time at the provided position.
        /// </summary>
        /// <param name="xPos">Position to draw the text at.</param>
        /// <param name="seconds">Time to display, in seconds.</param>
        /// <param name="minutes">If true the time will be displayed in minutes, otherwise in seconds.</param>
        private void DrawTime(int xPos, float seconds, bool minutes)
        {
            TimeSpan timeSpan = TimeSpan.FromSeconds(seconds);

            string timeString;
            if (minutes)
                timeString = timeSpan.TotalMinutes.ToString("#0") + ":" + timeSpan.Seconds.ToString("D2");
            else
                timeString = timeSpan.TotalSeconds.ToString("#0.00");

            Vector2I textBounds = GUIUtility.CalculateTextBounds(timeString, EditorBuiltin.DefaultFont,
                EditorStyles.DefaultFontSize);

            Vector2I textPosition = new Vector2I();
            textPosition.x = xPos - textBounds.x / 2;
            textPosition.y = TEXT_PADDING;

            canvas.DrawText(timeString, textPosition, EditorBuiltin.DefaultFont, Color.LightGray,
                EditorStyles.DefaultFontSize);
        }

        /// <summary>
        /// Draws one tick of the timeline, at the specified time.
        /// </summary>
        /// <param name="t">Time at which to draw the tick.</param>
        /// <param name="strength">Strength of the tick (determines size and color), in range [0, 1].</param>
        /// <param name="drawText">If true the text displaying the time will be drawn above this tick.</param>
        /// <param name="displayAsMinutes">Should the text drawn be displayed as minutes (if true), or seconds (false).
        ///                                Ignored if no text is drawn.</param>
        private void DrawTick(float t, float strength, bool drawText, bool displayAsMinutes)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + PADDING;

            // Draw tick
            Vector2I start = new Vector2I(xPos, height - (int)(tickHeight * strength));
            Vector2I end = new Vector2I(xPos, height);

            Color color = Color.LightGray;
            color.a *= strength;

            canvas.DrawLine(start, end, color);

            // Draw text if it fits
            if (drawText)
                DrawTime(xPos, t, displayAsMinutes);
        }

        /// <summary>
        /// Draws a vertical frame marker at the specified time.
        /// </summary>
        /// <param name="t">Time at which to draw the marker.</param>
        private void DrawFrameMarker(float t)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + PADDING;

            Vector2I start = new Vector2I(xPos, 0);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, Color.BansheeOrange);
        }

        /// <summary>
        /// Returns the range of times displayed by the timeline rounded to the multiple of FPS.
        /// </summary>
        /// <param name="padding">If true, extra range will be included to cover the right-most padding.</param>
        /// <returns>Time range rounded to a multiple of FPS.</returns>
        private float GetRange(bool padding = false)
        {
            float spf = 1.0f/fps;

            float range = rangeLength;
            if (padding)
            {
                float lengthPerPixel = rangeLength/drawableWidth;
                range += lengthPerPixel * PADDING;
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

            int numTickLevels = tickHandler.NumLevels;
            for (int i = numTickLevels - 1; i >= 0; i--)
            {
                bool drawText = i == 0;

                float[] ticks = tickHandler.GetTicks(i);
                float strength = tickHandler.GetLevelStrength(i);

                if (ticks.Length > 0)
                {
                    float valuePerTick = range/ticks.Length;
                    bool displayAsMinutes = TimeSpan.FromSeconds(valuePerTick).Minutes > 0;
                    for (int j = 0; j < ticks.Length; j++)
                        DrawTick(ticks[j], strength, drawText, displayAsMinutes);
                }
            }

            if (markedFrameIdx != -1)
            {
                int numFrames = (int)range * fps;
                float timePerFrame = range / numFrames;

                DrawFrameMarker(markedFrameIdx*timePerFrame);
            }
        }
    }

    /** @} */
}

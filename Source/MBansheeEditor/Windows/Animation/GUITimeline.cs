//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */

    // TODO DOC
    public class GUITimeline
    {
        public const int PADDING = 30;
        public const int OPTIMAL_TICK_WIDTH = 15;

        private const float LARGE_TICK_HEIGHT_PCT = 0.4f;
        private const float SMALL_TICK_HEIGHT_PCT = 0.2f;
        private const int TEXT_PADDING = 2;
        
        private int maxTextWidth;
        private int largeTickHeight;
        private int smallTickHeight;
        private int drawableWidth;
        private float rangeLength = 60.0f;
        private int minWidth = 0;

        private GUICanvas canvas;
        private int width;
        private int height;
        private int fps = 1;
        private int markedFrameIdx = -1;

        public int MinWidth
        {
            get { return minWidth; }
        }

        public GUITimeline(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            maxTextWidth = GUIUtility.CalculateTextBounds("99:999", EditorBuiltin.DefaultFont,
               EditorStyles.DefaultFontSize).x;

            SetSize(width, height);
        }

        public int GetFrame(Vector2I windowCoords)
        {
            Rect2I bounds = canvas.Bounds;

            if (windowCoords.x < (bounds.x + PADDING) || windowCoords.x >= (bounds.x + bounds.width - PADDING) ||
                windowCoords.y < bounds.y || windowCoords.y >= (bounds.y + bounds.height))
            {
                return -1;
            }

            Vector2I relativeCoords = windowCoords - new Vector2I(bounds.x + PADDING, bounds.y);

            float lengthPerPixel = rangeLength / drawableWidth;
            float time = relativeCoords.x * lengthPerPixel;

            return (int)(time * fps);
        }

        // Set to -1 to clear it
        public void SetMarkedFrame(int frameIdx)
        {
            markedFrameIdx = frameIdx;

            Rebuild();
        }

        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            largeTickHeight = (int)(height * LARGE_TICK_HEIGHT_PCT);
            smallTickHeight = (int)(height * SMALL_TICK_HEIGHT_PCT);
            drawableWidth = Math.Max(0, width - PADDING * 2);

            Rebuild();
        }

        public void SetRange(float length)
        {
            rangeLength = Math.Max(0.0f, length);

            Rebuild();
        }

        public void SetFPS(int fps)
        {
            this.fps = Math.Max(1, fps);

            Rebuild();
        }
        
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

        private void DrawLargeTick(float t, bool drawText, bool displayAsMinutes)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + PADDING;

            // Draw tick
            Vector2I start = new Vector2I(xPos, height - largeTickHeight);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, Color.LightGray);

            // Draw text if it fits
            if (drawText)
                DrawTime(xPos, t, displayAsMinutes);
        }

        private void DrawSmallTick(float t)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + PADDING;

            // Draw tick
            Vector2I start = new Vector2I(xPos, height - smallTickHeight);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, Color.LightGray);
        }

        private void DrawFrameMarker(float t)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + PADDING;

            Vector2I start = new Vector2I(xPos, 0);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, Color.Red);
        }

        // Returns range rounded to the nearest multiple of FPS
        private float GetRange()
        {
            float spf = 1.0f/fps;

            return ((int)rangeLength/spf) * spf;
        }

        private void Rebuild()
        {
            const int TEXT_SPACING = 10;
            canvas.Clear();

            // TODO - Transition between interval sizes more lightly (dynamically change tick height?)
            // TODO - Calculate min width
            // TODO - Time values change as width changes, keep them constant?

            float range = GetRange();

            int numFrames = (int)range * fps;
            float frameWidth = drawableWidth / (float)numFrames;
            
            int tickInterval = (int)Math.Max(1.0f, OPTIMAL_TICK_WIDTH / frameWidth);
            int largeTickInterval = tickInterval * 5;
            float largeTickWidth = frameWidth * largeTickInterval;

            float timePerFrame = range / numFrames;
            float timePerTick = timePerFrame*tickInterval;
            bool displayAsMinutes = TimeSpan.FromSeconds(timePerTick).Minutes > 0;

            int textInterval = MathEx.CeilToInt((maxTextWidth + TEXT_SPACING) / largeTickWidth);

            // Draw extra frames to prevent the out-of-frame ticks from popping in and out as range changes
            float extraWidth = PADDING + maxTextWidth / 2;
            numFrames += (int)(extraWidth / frameWidth);

            float t = 0.0f;
            for (int i = 0; i < numFrames; i++)
            {
                if (i%largeTickInterval == 0)
                {
                    int textIdx = i/largeTickInterval;
                    bool drawText = textIdx % textInterval == 0;

                    DrawLargeTick(t, drawText, displayAsMinutes);
                }
                else if (i%tickInterval == 0)
                    DrawSmallTick(t);

                // Move to next tick
                t += timePerFrame;
            }

            if (markedFrameIdx != -1)
                DrawFrameMarker(markedFrameIdx * timePerFrame);
        }
    }

    /** @} */
}

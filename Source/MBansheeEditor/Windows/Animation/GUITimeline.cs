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
        private const float LARGE_TICK_HEIGHT_PCT = 0.4f;
        private const float SMALL_TICK_HEIGHT_PCT = 0.2f;
        private const int PADDING = 30;
        private const int TEXT_PADDING = 2;
        private const int MIN_TICK_DISTANCE = 25;

        private int maxTextWidth;
        private int largeTickHeight;
        private int drawableWidth;
        private float rangeLength = 60.0f;

        private GUICanvas canvas;
        private int width;
        private int height;
        private int fps = 1;

        public GUITimeline(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            maxTextWidth = GUIUtility.CalculateTextBounds("99:999", EditorBuiltin.DefaultFont,
               EditorStyles.DefaultFontSize).x;

            SetSize(width, height);
        }

        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            largeTickHeight = (int)(height * LARGE_TICK_HEIGHT_PCT);
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

        private float CalcTickInterval()
        {
            const int OPTIMAL_TICK_COUNT = 10;
            float[] validIntervals =
            {
                0.001f, 0.005f, 0.010f, 0.025f, 0.050f, 0.100f, 0.250f, 0.500f, // Hundreds of a second
                1.0f, 5.0f, 10.0f, 30.0f, // Seconds
                60.0f, 120.0f, 300.0f, 600.0f, 1800.0f, 3600.0f // Minutes
            };

            float timePerFrame = 1.0f/fps;
            int bestIntervalIdx = 0;
            float bestDistance = float.MaxValue;

            for(int i = 0; i < validIntervals.Length; i++)
            {
                // Cannot choose an interval that would display ticks for below the frame-rate
                if (validIntervals[i] < timePerFrame)
                    continue;

                float tickCount = rangeLength/validIntervals[i];
                float distance = Math.Abs(tickCount - OPTIMAL_TICK_COUNT);
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestIntervalIdx = i;
                }
            }

            // If the draw area is too narrow, limit amount of ticks displayed so they aren't all clumped together
            int numTicks = MathEx.FloorToInt(rangeLength / validIntervals[bestIntervalIdx]) + 1;
            int spacePerTick = drawableWidth/numTicks;

            float bestInterval;
            if (spacePerTick < MIN_TICK_DISTANCE)
            {
                int maxTickCount = drawableWidth/MIN_TICK_DISTANCE;

                bool foundInterval = false;
                for (int i = bestIntervalIdx; i < validIntervals.Length; i++)
                {
                    float tickCount = rangeLength / validIntervals[i];
                    if (tickCount <= maxTickCount)
                    {
                        bestIntervalIdx = i;
                        foundInterval = true;
                        break;
                    }
                }

                // Haven't found a valid round interval, try more intervals
                if (!foundInterval)
                {
                    float currentInterval = validIntervals[validIntervals.Length - 1]*2;
                    while (true)
                    {
                        float tickCount = rangeLength / currentInterval;
                        if (tickCount <= maxTickCount)
                        {
                            bestInterval = currentInterval;
                            break;
                        }

                        currentInterval *= 2;
                    }
                }
                else
                    bestInterval = validIntervals[bestIntervalIdx];
            }
            else
                bestInterval = validIntervals[bestIntervalIdx];

            return bestInterval;
        }

        private void DrawTime(int xPos, float seconds, bool minutes)
        {
            TimeSpan timeSpan = TimeSpan.FromSeconds(seconds);

            string timeString;
            if (minutes)
            {
                timeString = timeSpan.TotalMinutes + ":" + timeSpan.Seconds.ToString("D2");
            }
            else
            {
                int hundredths = timeSpan.Milliseconds / 10;
                timeString = timeSpan.TotalSeconds + "." + hundredths.ToString("D3");
            }

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
            int xPos = (int)((t / rangeLength) * drawableWidth) + PADDING;

            // Draw tick
            Vector2I start = new Vector2I(xPos, height - largeTickHeight);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, Color.LightGray);

            // Draw text if it fits
            if (drawText)
                DrawTime(xPos, t, displayAsMinutes);
        }

        private void Rebuild()
        {
            const int TEXT_SPACING = 10;
            canvas.Clear();

            // TODO - Draw small ticks (don't forget to handle offset properly)
            // TODO - Transition between interval sizes more lightly (dynamically change tick height?)

            // Draw ticks
            float tickInterval = CalcTickInterval();

            // Draw extra ticks outside the visible width so they don't just pop-in/out as range increases/decreases
            float extraRange = PADDING + maxTextWidth / 2;
            int numTicks = MathEx.FloorToInt((rangeLength + extraRange) / tickInterval) + 1;
            bool displayAsMinutes = TimeSpan.FromSeconds(tickInterval).Minutes > 0;

            int tickWidth = (int) ((tickInterval/rangeLength)*drawableWidth);
            int textInterval = MathEx.CeilToInt((maxTextWidth + TEXT_SPACING)/ (float)tickWidth);

            float t = 0.0f;
            for (int i = 0; i < numTicks; i++)
            {
                bool drawText = i%textInterval == 0;

                DrawLargeTick(t, drawText, displayAsMinutes);

                // Move to next tick
                t += tickInterval;
            }
        }
    }

    /** @} */
}

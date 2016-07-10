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
        private const int MIN_TICK_DISTANCE = 10;

        private GUICanvas canvas;
        private int width;
        private int height;
        private float rangeStart = 0.0f;
        private float rangeEnd = 60.0f;
        private int fps = 1;

        public GUITimeline(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            SetSize(width, height);
        }

        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            Rebuild();
        }

        public void SetRange(float start, float end)
        {
            rangeStart = start;
            rangeEnd = end;

            Rebuild();
        }

        public void SetFPS(int fps)
        {
            this.fps = fps;

            Rebuild();
        }

        private void Rebuild()
        {
            canvas.Clear();

            // TODO - Calculate interval sizes based on set range, width and FPS
            //      - Dynamically change tick heights?

            int drawableWidth = Math.Max(0, width - PADDING * 2);
            float rangeLength = rangeEnd - rangeStart;
            float numSmallTicksPerLarge = 5.0f;

            int totalNumFrames = MathEx.FloorToInt(rangeLength*fps);
            int numVisibleTicks = Math.Min(totalNumFrames, MathEx.FloorToInt(drawableWidth / (float) MIN_TICK_DISTANCE));

            float smallTickInterval = rangeLength / numVisibleTicks;
            float largeTickInterval = smallTickInterval * numSmallTicksPerLarge;

            float offsetLarge = MathEx.CeilToInt(rangeStart / largeTickInterval) * largeTickInterval - rangeStart;
            float offsetSmall = MathEx.CeilToInt(rangeStart / smallTickInterval) * smallTickInterval - rangeStart;

            int largeTickHeight = (int)(height * LARGE_TICK_HEIGHT_PCT);
            int smallTickHeight = (int)(height * SMALL_TICK_HEIGHT_PCT);

            bool drawSmallTicks = true; // TODO

            float t = offsetSmall;
            for (int i = 0; i < numVisibleTicks; i++)
            {
                float distanceToLargeTick = MathEx.CeilToInt(t / largeTickInterval) * largeTickInterval - t;
                if (MathEx.ApproxEquals(distanceToLargeTick, 0.0f))
                {
                    int xPos = (int)((t/rangeLength)* drawableWidth) + PADDING;

                    Vector2I start = new Vector2I(xPos, height - largeTickHeight);
                    Vector2I end = new Vector2I(xPos, height);

                    canvas.DrawLine(start, end, Color.LightGray);

                    TimeSpan intervalSpan = TimeSpan.FromSeconds(largeTickInterval);
                    TimeSpan timeSpan = TimeSpan.FromSeconds(rangeStart + t);

                    string timeString;
                    if(intervalSpan.Minutes > 0)
                        timeString = timeSpan.ToString(@"m\:ss");
                    else
                        timeString = timeSpan.ToString(@"ss\:fff");

                    Vector2I textBounds = GUIUtility.CalculateTextBounds(timeString, EditorBuiltin.DefaultFont, 
                        EditorStyles.DefaultFontSize);

                    Vector2I textPosition = new Vector2I();
                    textPosition.x = xPos - textBounds.x/2;
                    textPosition.y = TEXT_PADDING;

                    canvas.DrawText(timeString, textPosition, EditorBuiltin.DefaultFont, Color.LightGray, 
                        EditorStyles.DefaultFontSize);
                }
                else
                {
                    if (drawSmallTicks)
                    {
                        int xPos = (int)((t / rangeLength) * drawableWidth) + PADDING;

                        Vector2I start = new Vector2I(xPos, height - smallTickHeight);
                        Vector2I end = new Vector2I(xPos, height);

                        canvas.DrawLine(start, end, Color.LightGray);
                    }
                }

                t += smallTickInterval;
            }
        }
    }

    /** @} */
}

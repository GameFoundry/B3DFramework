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
        private const float LARGE_TICK_HEIGHT_PCT = 0.5f;
        private const float SMALL_TICK_HEIGHT_PCT = 0.25f;

        private GUICanvas canvas;
        private int width;
        private int height;
        private float rangeStart = 0.0f;
        private float rangeEnd = 60.0f;
        private int fps = 60;

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

            canvas.DrawLine(new Vector2I(50, 20), new Vector2I(50, 40), Color.White);
            canvas.DrawLine(new Vector2I(100, 20), new Vector2I(100, 40), Color.White);

            return;

            // TODO - Calculate interval sizes based on set range, width and FPS
            //      - Dynamically change tick heights?
            //      - Draw text (and convert seconds to minutes/hours as needed)

            float largeTickInterval = 50.0f; // TODO - Must be multiple of small tick interval
            float smallTickInterval = 10.0f; // TODO

            float offsetLarge = MathEx.CeilToInt(rangeStart / largeTickInterval) * largeTickInterval - rangeStart;
            float offsetSmall = MathEx.CeilToInt(rangeStart / smallTickInterval) * smallTickInterval - rangeStart;

            int largeTickHeight = (int)(height * LARGE_TICK_HEIGHT_PCT);
            int smallTickHeight = (int)(height * SMALL_TICK_HEIGHT_PCT);

            bool drawSmallTicks = true; // TODO

            float length = rangeEnd - rangeStart;
            for (float t = offsetSmall; t <= length; t += smallTickInterval)
            {
                Debug.Log(t + " - " + length + " - " + width);

                float distanceToLargeTick = MathEx.CeilToInt(t / largeTickInterval) * largeTickInterval - t;
                if (MathEx.ApproxEquals(distanceToLargeTick, 0.0f))
                {
                    int xPos = (int)((t/length)*width);

                    Vector2I start = new Vector2I(xPos, height - largeTickHeight);
                    Vector2I end = new Vector2I(xPos, height);

                    canvas.DrawLine(start, end, Color.DarkGray);

                    TimeSpan intervalSpan = TimeSpan.FromSeconds(largeTickInterval);
                    TimeSpan timeSpan = TimeSpan.FromSeconds(rangeStart + t);

                    string timeString;
                    if(intervalSpan.Minutes > 0)
                        timeString = timeSpan.ToString(@"m\:ss");
                    else
                        timeString = timeSpan.ToString(@"ss\:fff");

                    Vector2I textBounds = GUIUtility.CalculateTextBounds(timeString, Builtin.DefaultFont, 
                        EditorStyles.DefaultFontSize);

                    Vector2I textPosition = new Vector2I();
                    textPosition.x = -textBounds.x/2;

                    //canvas.DrawText(timeString, textPosition, Builtin.DefaultFont, Color.DarkGray, 
                    //    EditorStyles.DefaultFontSize);
                }
                else
                {
                    if (drawSmallTicks)
                    {
                        int xPos = (int)((t / length) * width);

                        Vector2I start = new Vector2I(xPos, height - smallTickHeight);
                        Vector2I end = new Vector2I(xPos, height);

                        canvas.DrawLine(start, end, Color.LightGray);
                    }
                }
            }
        }
    }

    /** @} */
}

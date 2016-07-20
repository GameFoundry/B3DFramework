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

        private const float TICK_HEIGHT_PCT = 0.4f;
        private const int TEXT_PADDING = 2;
        
        private int tickHeight;
        private int drawableWidth;
        private float rangeLength = 60.0f;

        private GUICanvas canvas;
        private GUITicks tickHandler;
        private int width;
        private int height;
        private int fps = 1;
        private int markedFrameIdx = -1;

        public GUITimeline(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            tickHandler = new GUITicks(GUITickStepType.Time);

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

            tickHeight = (int)(height * TICK_HEIGHT_PCT);
            drawableWidth = Math.Max(0, width - PADDING * 2);

            tickHandler.SetRange(0.0f, GetRange(), drawableWidth);

            Rebuild();
        }

        public void SetRange(float length)
        {
            rangeLength = Math.Max(0.0f, length);

            tickHandler.SetRange(0.0f, GetRange(), drawableWidth);

            Rebuild();
        }

        public void SetFPS(int fps)
        {
            this.fps = Math.Max(1, fps);

            tickHandler.SetRange(0.0f, GetRange(), drawableWidth);

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

        private void DrawTick(float t, float strength, bool drawText, bool displayAsMinutes)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + PADDING;

            strength = MathEx.Clamp01(strength);

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

        private void DrawFrameMarker(float t)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + PADDING;

            Vector2I start = new Vector2I(xPos, 0);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, Color.BansheeOrange);
        }

        // Returns range rounded to the nearest multiple of FPS
        private float GetRange()
        {
            float spf = 1.0f/fps;

            return ((int)rangeLength/spf) * spf;
        }

        private void Rebuild()
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

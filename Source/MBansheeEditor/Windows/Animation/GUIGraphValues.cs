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
    /// Renders a vertical value display that may be used as a side-bar for a graph display. User can set the range of the
    /// values to display, as well as its physical dimensions.
    /// </summary>
    internal class GUIGraphValues
    {
        private static readonly Color COLOR_TRANSPARENT_LIGHT_GRAY = 
            new Color(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 0.5f);

        private GUIGraphTicks tickHandler;
        private GUICanvas canvas;

        private int width = 20;
        private int height = 20;
        private float rangeStart = -1.0f;
        private float rangeEnd = 1.0f;

        /// <summary>
        /// Constructs a new value display and adds it to the specified layout.
        /// </summary>
        /// <param name="layout">Layout to add the GUI element to.</param>
        /// <param name="width">Width of the timeline in pixels.</param>
        /// <param name="height">Height of the timeline in pixels.</param>
        public GUIGraphValues(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            tickHandler = new GUIGraphTicks();

            SetSize(width, height);
        }

        /// <summary>
        /// Sets the physical size onto which to draw the value display.
        /// </summary>
        /// <param name="width">Width in pixels.</param>
        /// <param name="height">Height in pixels.</param>
        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            tickHandler.SetRange(rangeStart, rangeEnd, height);
        }

        /// <summary>
        /// Sets the range of values to display.
        /// </summary>
        /// <param name="start">Minimum value to display.</param>
        /// <param name="end">Maximum value to display.</param>
        public void SetRange(float start, float end)
        {
            if (start > end)
            {
                float temp = start;
                start = end;
                end = temp;
            }

            rangeStart = start;
            rangeEnd = end;

            tickHandler.SetRange(rangeStart, rangeEnd, height);
        }

        /// <summary>
        /// Draws text displaying the value at the provided position.
        /// </summary>
        /// <param name="yPos">Position to draw the text at.</param>
        /// <param name="value">Value to display.</param>
        /// <param name="above">If true the text will be displayed above the provided position, otherwise below.</param>
        private void DrawValue(int yPos, float value, bool above)
        {
            int exponent = MathEx.FloorToInt(MathEx.Log10(MathEx.Abs(value)));
            int maxDecimalPoints = MathEx.Max(0, 1 - exponent);

            string valueString = value.ToString("F" + maxDecimalPoints);

            Vector2I textBounds = GUIUtility.CalculateTextBounds(valueString, EditorBuiltin.DefaultFont,
                EditorStyles.DefaultFontSize);

            Vector2I textPosition = new Vector2I();
            textPosition.x = width - textBounds.x;

            if (above)
                textPosition.y = yPos - textBounds.y;
            else // Below
            {
                const int PADDING = 3; // So the text doesn't touch the tick
                textPosition.y = yPos + PADDING;
            }
            canvas.DrawText(valueString, textPosition, EditorBuiltin.DefaultFont, COLOR_TRANSPARENT_LIGHT_GRAY,
                EditorStyles.DefaultFontSize);
        }

        /// <summary>
        /// Rebuilds the internal GUI elements. Should be called whenever timeline properties change.
        /// </summary>
        public void Rebuild()
        {
            canvas.Clear();

            int heightOffset = height/2;
            float pixelsPerHeight;

            if (rangeEnd != rangeStart)
                pixelsPerHeight = height/(rangeEnd - rangeStart);
            else
                pixelsPerHeight = 0;

            float yOffset = rangeStart + (rangeEnd - rangeStart)*0.5f;

            int numTickLevels = tickHandler.NumLevels;
            for (int i = numTickLevels - 1; i >= 0; i--)
            {
                float[] ticks = tickHandler.GetTicks(i);
                float strength = tickHandler.GetLevelStrength(i);
               
                if (ticks.Length > 0)
                {
                    float valuePerTick = (rangeEnd - rangeStart)/ticks.Length;

                    for (int j = 0; j < ticks.Length; j++)
                    {
                        int yPos = (int) ((ticks[j] - yOffset) * pixelsPerHeight);
                        yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                        Vector2I start = new Vector2I(0, yPos);
                        Vector2I end = new Vector2I((int) (width*strength), yPos);

                        Color color = COLOR_TRANSPARENT_LIGHT_GRAY;
                        color.a *= strength;

                        canvas.DrawLine(start, end, color);

                        // Draw text for the highest level ticks
                        if (i == 0)
                            DrawValue(yPos, ticks[j], ticks[j] <= 0.0f);
                    }
                }
            }
        }
    }

    /** @} */
}

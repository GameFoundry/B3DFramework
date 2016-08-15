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
    public class GUIGraphTime : GUITimelineBase
    {
        private const float TICK_HEIGHT_PCT = 0.4f;
        private const int TEXT_PADDING = 2;
        
        private GUIGraphTicks tickHandler;

        /// <summary>
        /// Constructs a new timeline and adds it to the specified layout.
        /// </summary>
        /// <param name="layout">Layout to add the timeline GUI to.</param>
        /// <param name="width">Width of the timeline in pixels.</param>
        /// <param name="height">Height of the timeline in pixels.</param>
        public GUIGraphTime(GUILayout layout, int width, int height)
            :base(layout, width, height)
        {
            tickHandler = new GUIGraphTicks(GUITickStepType.Time);
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
            int xPos = (int)(((t - rangeOffset) / GetRange()) * drawableWidth) + PADDING;

            // Draw tick
            float tickHeight = (int)(height * TICK_HEIGHT_PCT);
            Vector2I start = new Vector2I(xPos, height - (int)(tickHeight * strength));
            Vector2I end = new Vector2I(xPos, height);

            Color color = Color.LightGray;
            color.a *= strength;

            canvas.DrawLine(start, end, color);

            // Draw text if it fits
            if (drawText)
                DrawTime(xPos, t, displayAsMinutes);
        }

        /// <inheritdoc/>
        public override void Rebuild()
        {
            canvas.Clear();
            tickHandler.SetRange(rangeOffset, rangeOffset + GetRange(true), drawableWidth + PADDING);

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

            DrawFrameMarker();
        }
    }

    /** @} */
}

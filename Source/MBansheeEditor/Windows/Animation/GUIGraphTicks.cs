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
    /// Determines how should ticks reported by <see cref="GUIGraphTicks"/> be distributed. 
    /// </summary>
    internal enum GUITickStepType
    {
        /// <summary>
        /// Ticks represent time values (Multiples of 60).
        /// </summary>
        Time,
        /// <summary>
        /// Ticks represent generic values (Multiples of 10).
        /// </summary>
        Generic
    }

    /// <summary>
    /// Generates a set of locations that can be used for rendering ticks on a graph. As input the class takes valid range,
    /// size of the area the ticks will be displayed on, type of ticks and minimum/maximum spacing and outputs a set of
    /// coordinates along which ticks should be positioned. Ticks are reported as multiple separate levels with different
    /// strengths, depending on how close their distribution is to the upper valid range.
    /// </summary>
    internal class GUIGraphTicks
    {
        private int pixelRange = 100;
        private float valueRangeStart = 0.0f;
        private float valueRangeEnd = 1.0f;

        private int minTickSpacingPx = 5;
        private int maxTickSpacingPx = 30;

        private float[] validSteps = new [] { 1.0f };
        private float[] levelStrengths = new[] { 1.0f };
        private int numLevels = 1;
        private int maxLevel = 0;

        public int NumLevels { get { return numLevels; } }

        /// <summary>
        /// Contructs a new tick generating object.
        /// </summary>
        /// <param name="stepType">Determines how will ticks be distributed.</param>
        internal GUIGraphTicks(GUITickStepType stepType = GUITickStepType.Generic)
        {
            if(stepType == GUITickStepType.Generic)
                SetGenericSteps();
            else
                SetTimeSteps();

            Rebuild();
        }

        /// <summary>
        /// Sets the range which ticks are to be displayed for, and the range along which the ticks will be displayed.
        /// </summary>
        /// <param name="valueRangeStart">Start of the range the ticks are to display.</param>
        /// <param name="valueRangeEnd">End of the range the ticks are to display.</param>
        /// <param name="pixelRange">Width or height on which the ticks will be rendered. In pixels.</param>
        internal void SetRange(float valueRangeStart, float valueRangeEnd, int pixelRange)
        {
            this.valueRangeStart = valueRangeStart;
            this.valueRangeEnd = valueRangeEnd;
            this.pixelRange = pixelRange;

            Rebuild();
        }

        /// <summary>
        /// Sets valid spacing between two ticks. Tick strength will be determined by how far away are they from either
        /// end of this range.
        /// </summary>
        /// <param name="minPx">Minimum spacing between two ticks, in pixels.</param>
        /// <param name="maxPx">Maximum spacing between two ticks, in pixels.</param>
        internal void SetTickSpacing(int minPx, int maxPx)
        {
            minTickSpacingPx = minPx;
            maxTickSpacingPx = maxPx;

            Rebuild();
        }

        /// <summary>
        /// Returns the strength of a particular tick level. Levels are ordered in descending order of strength (level 0 is
        /// the strongest).
        /// </summary>
        /// <param name="level">Level for which to retrieve the strength. Must not be larger than 
        ///                     <see cref="NumLevels"/> - 1.</param>
        /// <returns>Strength of the ticks at this level, in range [0, 1].</returns>
        internal float GetLevelStrength(int level)
        {
            if (level < 0 || level >= numLevels)
                return 0.0f;

            return MathEx.Clamp01(levelStrengths[maxLevel + level]);
        }

        /// <summary>
        /// Returns positions of all ticks of the provided level. The ticks will be within the range provided to
        /// <see cref="SetRange"/>.
        /// </summary>
        /// <param name="level">Level for which to retrieve the positions. Must not be larger than 
        ///                     <see cref="NumLevels"/> - 1.</param>
        /// <returns>Positions of all ticks of the provided level.</returns>
        internal float[] GetTicks(int level)
        {
            if (level < 0 || level >= numLevels)
                return new float[0];

            float step = validSteps[maxLevel + level];

            // Round up and down so we get one extra tick on either side (outside of value range)
            // (Useful when rendering text above the ticks, so the text doesn't just pop-in when the tick appears, instead
            // it is slowly clipped-in.)
            int startTick = MathEx.CeilToInt(valueRangeStart / step);
            int endTick = MathEx.FloorToInt(valueRangeEnd / step);

            int numTicks = endTick - startTick + 1;

            float[] ticks = new float[numTicks];
            for (int i = startTick; i <= endTick; i++)
                ticks[i - startTick] = i*step;

            return ticks;
        }

        /// <summary>
        /// Rebuilds the tick positions and strengths after some relevant parameter changes.
        /// </summary>
        private void Rebuild()
        {
            levelStrengths = new float[validSteps.Length];
            maxLevel = 0;

            float valueRange = valueRangeEnd - valueRangeStart;
            int tickSpacing = maxTickSpacingPx - minTickSpacingPx;
            int i = 0;
            for (; i < validSteps.Length; i++)
            {
                float tickSpacingPx = (validSteps[i]/valueRange) * pixelRange;
                levelStrengths[i] = (tickSpacingPx - minTickSpacingPx)/tickSpacing;

                if (levelStrengths[i] > 1.0f)
                    maxLevel = i;
                else if (levelStrengths[i] < 0.0f)
                    break;
            }

            if (i > 0)
                numLevels = i - maxLevel;
            else
                numLevels = 0;
        }

        /// <summary>
        /// Sets tick steps corresponding to time values. This will split the ticks into intervals relevant for displaying
        /// times.
        /// </summary>
        private void SetTimeSteps()
        {
            validSteps = new float[]
            {
                3600.0f, 1800.0f, 600.0f, 300.0f,
                60.0f, 30.0f, 10.0f, 5.0f,
                1.0f, 0.5f, 0.25f, 0.1f, 0.05f, 0.01f
            };
        }

        /// <summary>
        /// Sets tick steps corresponding to generic values (as opposed to displaying time values).
        /// </summary>
        private void SetGenericSteps()
        {
            float minStep = 0.0000001f;
            int numSteps = 15;

            validSteps = new float[15 * 2];
            for (int i = numSteps - 1; i >= 0; i--)
            {
                validSteps[i * 2 + 1] = minStep;
                validSteps[i * 2 + 0] = minStep * 5;

                minStep *= 10.0f;
            }
        }
    }

    /** @} */
}

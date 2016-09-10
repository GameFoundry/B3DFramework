//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */

    /// <summary>
    /// Draws one or multiple curves over the specified physical area. User can specify horizontal and vertical range to
    /// display, as well as physical size of the GUI area.
    /// </summary>
    internal class GUICurveDrawing : GUITimelineBase
    {
        private const int LINE_SPLIT_WIDTH = 2;
        private const int TANGENT_LINE_DISTANCE = 30;
        private static readonly Color COLOR_MID_GRAY = new Color(90.0f / 255.0f, 90.0f / 255.0f, 90.0f / 255.0f, 1.0f);
        private static readonly Color COLOR_DARK_GRAY = new Color(40.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 1.0f);

        private CurveDrawInfo[] curveInfos;
        private bool[][] selectedKeyframes;

        private float yRange = 20.0f;
        private float yOffset;

        private GUIGraphTicks tickHandler;

        /// <summary>
        /// Creates a new curve drawing GUI element.
        /// </summary>
        /// <param name="layout">Layout into which to add the GUI element.</param>
        /// <param name="width">Width of the element in pixels.</param>
        /// <param name="height">Height of the element in pixels.</param>
        /// <param name="curveInfos">Initial set of curves to display. </param>
        public GUICurveDrawing(GUILayout layout, int width, int height, CurveDrawInfo[] curveInfos)
            :base(layout, width, height)
        {
            tickHandler = new GUIGraphTicks(GUITickStepType.Time);
            this.curveInfos = curveInfos;
            
            ClearSelectedKeyframes(); // Makes sure the array is initialized
        }

        /// <summary>
        /// Change the set of curves to display.
        /// </summary>
        /// <param name="curveInfos">New set of curves to draw on the GUI element.</param>
        public void SetCurves(CurveDrawInfo[] curveInfos)
        {
            this.curveInfos = curveInfos;
        }
        
        /// <summary>
        /// Changes the visible range that the GUI element displays.
        /// </summary>
        /// <param name="xRange">Range of the horizontal area. Displayed area will range from [0, xRange].</param>
        /// <param name="yRange">Range of the vertical area. Displayed area will range from 
        ///                      [-yRange * 0.5, yRange * 0.5]</param>
        public void SetRange(float xRange, float yRange)
        {
            SetRange(xRange);
            this.yRange = yRange;
        }

        /// <summary>
        /// Returns the offset at which the displayed timeline values start at.
        /// </summary>
        /// <param name="offset">Value to start the timeline values at.</param>
        public void SetOffset(Vector2 offset)
        {
            SetOffset(offset.x);
            yOffset = offset.y;
        }
        
        /// <summary>
        /// Marks the specified key-frame as selected, changing the way it is displayed.
        /// </summary>
        /// <param name="keyframeRef">Keyframe reference containing the curve and keyframe index.</param>
        /// <param name="selected">True to select it, false to deselect it.</param>
        public void SelectKeyframe(KeyframeRef keyframeRef, bool selected)
        {
            if (selectedKeyframes == null)
                return;

            if (keyframeRef.curveIdx < 0 || keyframeRef.curveIdx >= selectedKeyframes.Length)
                return;

            if (keyframeRef.keyIdx < 0 || keyframeRef.keyIdx >= selectedKeyframes[keyframeRef.curveIdx].Length)
                return;

            selectedKeyframes[keyframeRef.curveIdx][keyframeRef.keyIdx] = selected;
        }

        /// <summary>
        /// Clears any key-frames that were marked as selected.
        /// </summary>
        public void ClearSelectedKeyframes()
        {
            selectedKeyframes = new bool[curveInfos.Length][];

            for (int i = 0; i < curveInfos.Length; i++)
            {
                KeyFrame[] keyframes = curveInfos[i].curve.KeyFrames;
                selectedKeyframes[i] = new bool[keyframes.Length];
            }
        }

        /// <summary>
        /// Attempts to find a keyframe under the provided coordinates.
        /// </summary>
        /// <param name="pixelCoords">Coordinates relative to this GUI element in pixels.</param>
        /// <param name="keyframe">Output object containing keyframe index and index of the curve it belongs to. Only valid
        ///                        if method returns true.</param>
        /// <returns>True if there is a keyframe under the coordinates, false otherwise.</returns>
        public bool FindKeyFrame(Vector2I pixelCoords, out KeyframeRef keyframe)
        {
            keyframe = new KeyframeRef();

            float nearestDistance = float.MaxValue;
            for (int i = 0; i < curveInfos.Length; i++)
            {
                EdAnimationCurve curve = curveInfos[i].curve;
                KeyFrame[] keyframes = curve.KeyFrames;

                for (int j = 0; j < keyframes.Length; j++)
                {
                    Vector2 keyframeCurveCoords = new Vector2(keyframes[j].time, keyframes[j].value);
                    Vector2I keyframeCoords = CurveToPixelSpace(keyframeCurveCoords);

                    float distanceToKey = Vector2I.Distance(pixelCoords, keyframeCoords);
                    if (distanceToKey < nearestDistance)
                    {
                        nearestDistance = distanceToKey;
                        keyframe.keyIdx = j;
                        keyframe.curveIdx = i;
                    }
                }
            }

            // We're not near any keyframe
            if (nearestDistance > 5.0f)
                return false;

            return true;
        }

        /// <summary>
        /// Attempts to find a a tangent handle under the provided coordinates.
        /// </summary>
        /// <param name="pixelCoords">Coordinates relative to this GUI element in pixels.</param>
        /// <param name="tangent">Output object containing keyframe information and tangent type. Only valid if method
        ///                       returns true.</param>
        /// <returns>True if there is a tangent handle under the coordinates, false otherwise.</returns>
        public bool FindTangent(Vector2I pixelCoords, out TangentRef tangent)
        {
            tangent = new TangentRef();

            float nearestDistance = float.MaxValue;
            for (int i = 0; i < curveInfos.Length; i++)
            {
                EdAnimationCurve curve = curveInfos[i].curve;
                KeyFrame[] keyframes = curve.KeyFrames;

                for (int j = 0; j < keyframes.Length; j++)
                {
                    if (!IsSelected(i, j))
                        continue;

                    TangentMode tangentMode = curve.TangentModes[j];

                    if (IsTangentDisplayed(tangentMode, TangentType.In))
                    {
                        Vector2I tangentCoords = GetTangentPosition(keyframes[j], TangentType.In);

                        float distanceToHandle = Vector2I.Distance(pixelCoords, tangentCoords);
                        if (distanceToHandle < nearestDistance)
                        {
                            nearestDistance = distanceToHandle;
                            tangent.keyframeRef.keyIdx = j;
                            tangent.keyframeRef.curveIdx = i;
                            tangent.type = TangentType.In;
                        }
;                    }

                    if (IsTangentDisplayed(tangentMode, TangentType.Out))
                    {
                        Vector2I tangentCoords = GetTangentPosition(keyframes[j], TangentType.Out);

                        float distanceToHandle = Vector2I.Distance(pixelCoords, tangentCoords);
                        if (distanceToHandle < nearestDistance)
                        {
                            nearestDistance = distanceToHandle;
                            tangent.keyframeRef.keyIdx = j;
                            tangent.keyframeRef.curveIdx = i;
                            tangent.type = TangentType.Out;
                        }
                    }
                }
            }

            // We're not near any keyframe
            if (nearestDistance > 5.0f)
                return false;

            return true;
        }
        
        /// <summary>
        /// Converts pixel coordinates into coordinates in curve space.
        /// </summary>
        /// <param name="pixelCoords">Coordinates relative to this GUI element, in pixels.</param>
        /// <param name="curveCoords">Curve coordinates within the range as specified by <see cref="SetRange"/>. Only
        ///                           valid when function returns true.</param>
        /// <param name="padding">Determines should coordinates over padding be registered.</param>
        /// <returns>True if the coordinates are within the curve area, false otherwise.</returns>
        public bool PixelToCurveSpace(Vector2I pixelCoords, out Vector2 curveCoords, bool padding = false)
        {
            Rect2I bounds = canvas.Bounds;

            bool outsideHorizontal;
            if (padding)
                outsideHorizontal = pixelCoords.x < bounds.x || pixelCoords.x >= (bounds.x + bounds.width);
            else
                outsideHorizontal = pixelCoords.x < (bounds.x + PADDING) || pixelCoords.x >= (bounds.x + bounds.width - PADDING);

            // Check if outside of curve drawing bounds
            if (outsideHorizontal || pixelCoords.y < bounds.y || pixelCoords.y >= (bounds.y + bounds.height))
            {
                curveCoords = new Vector2();
                return false;
            }

            // Find time and value of the place under the coordinates
            Vector2I relativeCoords = pixelCoords - new Vector2I(bounds.x + PADDING, bounds.y);

            float lengthPerPixel = GetRange() / drawableWidth;
            float heightPerPixel = yRange / height;

            float centerOffset = yRange / 2.0f;

            float t = rangeOffset + relativeCoords.x * lengthPerPixel;
            float value = yOffset + centerOffset - relativeCoords.y * heightPerPixel;

            curveCoords = new Vector2();
            curveCoords.x = t;
            curveCoords.y = value;

            return true;
        }

        /// <summary>
        /// Converts coordinate in curve space (time, value) into pixel coordinates relative to this element's origin.
        /// </summary>
        /// <param name="curveCoords">Time and value of the location to convert.</param>
        /// <returns>Coordinates relative to this element's origin, in pixels.</returns>
        public Vector2I CurveToPixelSpace(Vector2 curveCoords)
        {
            int heightOffset = height / 2; // So that y = 0 is at center of canvas

            Vector2 relativeCurveCoords = curveCoords - new Vector2(rangeOffset, yOffset);

            Vector2I pixelCoords = new Vector2I();
            pixelCoords.x = (int)((relativeCurveCoords.x / GetRange()) * drawableWidth) + PADDING;
            pixelCoords.y = heightOffset - (int)((relativeCurveCoords.y / yRange) * height);

            return pixelCoords;
        }

        /// <summary>
        /// Generates a unique color based on the provided index.
        /// </summary>
        /// <param name="idx">Index to use for generating a color. Should be less than 30 in order to guarantee reasonably
        /// different colors.</param>
        /// <returns>Unique color.</returns>
        public static Color GetUniqueColor(int idx)
        {
            const int COLOR_SPACING = 359 / 15;

            float hue = ((idx * COLOR_SPACING) % 359) / 359.0f;
            return Color.HSV2RGB(new Color(hue, 175.0f / 255.0f, 175.0f / 255.0f));
        }

        /// <summary>
        /// Draws a vertical frame marker on the curve area.
        /// </summary>
        /// <param name="t">Time at which to draw the marker.</param>
        /// <param name="color">Color with which to draw the marker.</param>
        /// <param name="onTop">Determines should the marker be drawn above or below the curve.</param>
        private void DrawFrameMarker(float t, Color color, bool onTop)
        {
            int xPos = (int)(((t - rangeOffset) / GetRange()) * drawableWidth) + PADDING;

            Vector2I start = new Vector2I(xPos, 0);
            Vector2I end = new Vector2I(xPos, height);

            byte depth;
            if (onTop)
                depth = 110;
            else
                depth = 128;

            canvas.DrawLine(start, end, color, depth);
        }

        /// <summary>
        /// Draws a horizontal line representing the line at y = 0.
        /// </summary>
        private void DrawCenterLine()
        {
            Vector2I center = CurveToPixelSpace(new Vector2(0.0f, 0.0f));

            Vector2I start = new Vector2I(0, center.y);
            Vector2I end = new Vector2I(width, center.y);

            canvas.DrawLine(start, end, COLOR_DARK_GRAY);
        }

        /// <summary>
        /// Draws a diamond shape of the specified size at the coordinates.
        /// </summary>
        /// <param name="center">Position at which to place the diamond's center, in pixel coordinates.</param>
        /// <param name="size">Determines number of pixels to extend the diamond in each direction.</param>
        /// <param name="innerColor">Color of the diamond's background.</param>
        /// <param name="outerColor">Color of the diamond's outline.</param>
        private void DrawDiamond(Vector2I center, int size, Color innerColor, Color outerColor)
        {
            Vector2I a = new Vector2I(center.x - size, center.y);
            Vector2I b = new Vector2I(center.x, center.y - size);
            Vector2I c = new Vector2I(center.x + size, center.y);
            Vector2I d = new Vector2I(center.x, center.y + size);

            // Draw diamond shape
            Vector2I[] linePoints = new Vector2I[] { a, b, c, d, a };
            Vector2I[] trianglePoints = new Vector2I[] { b, c, a, d };

            canvas.DrawTriangleStrip(trianglePoints, innerColor, 101);
            canvas.DrawPolyLine(linePoints, outerColor, 100);
    }

        /// <summary>
        /// Draws a keyframe a the specified time and value.
        /// </summary>
        /// <param name="t">Time to draw the keyframe at.</param>
        /// <param name="y">Y value to draw the keyframe at.</param>
        /// <param name="selected">Determines should the keyframe be drawing using the selected color scheme, or normally.
        ///                        </param>
        private void DrawKeyframe(float t, float y, bool selected)
        {
            Vector2I pixelCoords = CurveToPixelSpace(new Vector2(t, y));

            if (selected)
                DrawDiamond(pixelCoords, 3, Color.White, Color.BansheeOrange);
            else
                DrawDiamond(pixelCoords, 3, Color.White, Color.Black);
        }

        /// <summary>
        /// Draws zero, one or two tangents for the specified keyframe. Whether tangents are drawn depends on the provided
        /// mode.
        /// </summary>
        /// <param name="keyFrame">Keyframe to draw the tangents for.</param>
        /// <param name="tangentMode">Type of tangents in the keyframe.</param>
        private void DrawTangents(KeyFrame keyFrame, TangentMode tangentMode)
        {
            Vector2I keyframeCoords = CurveToPixelSpace(new Vector2(keyFrame.time, keyFrame.value));

            if (IsTangentDisplayed(tangentMode, TangentType.In))
            {
                Vector2I tangentCoords = GetTangentPosition(keyFrame, TangentType.In);

                canvas.DrawLine(keyframeCoords, tangentCoords, Color.LightGray);
                DrawDiamond(tangentCoords, 2, Color.Green, Color.Black);
            }

            if (IsTangentDisplayed(tangentMode, TangentType.Out))
            {
                Vector2I tangentCoords = GetTangentPosition(keyFrame, TangentType.Out);

                canvas.DrawLine(keyframeCoords, tangentCoords, Color.LightGray);
                DrawDiamond(tangentCoords, 2, Color.Green, Color.Black);
            }
        }

        /// <summary>
        /// Returns the position of the tangent, in element's pixel space.
        /// </summary>
        /// <param name="keyFrame">Keyframe that the tangent belongs to.</param>
        /// <param name="type">Which tangent to retrieve the position for.</param>
        /// <returns>Position of the tangent, relative to the this GUI element's origin, in pixels.</returns>
        private Vector2I GetTangentPosition(KeyFrame keyFrame, TangentType type)
        {
            Vector2I position = CurveToPixelSpace(new Vector2(keyFrame.time, keyFrame.value));

            Vector2 normal;
            if (type == TangentType.In)
                normal = -EdAnimationCurve.TangentToNormal(keyFrame.inTangent);
            else
                normal = EdAnimationCurve.TangentToNormal(keyFrame.outTangent);

            // X/Y ranges aren't scaled 1:1, adjust normal accordingly
            normal.x /= GetRange();
            normal.y /= yRange;
            normal = Vector2.Normalize(normal);

            // Convert normal (in percentage) to pixel values
            Vector2I offset = new Vector2I((int)(normal.x * TANGENT_LINE_DISTANCE),
                    (int)(-normal.y * TANGENT_LINE_DISTANCE));

            return position + offset;
        }

        /// <summary>
        /// Checks if the tangent should be displayed, depending on the active tangent mode.
        /// </summary>
        /// <param name="mode">Tangent mode for the keyframe.</param>
        /// <param name="type">Which tangent to check for.</param>
        /// <returns>True if the tangent should be displayed.</returns>
        private bool IsTangentDisplayed(TangentMode mode, TangentType type)
        {
            if (mode == TangentMode.Auto)
                return false;
            else if (mode == TangentMode.Free)
                return true;

            if (type == TangentType.In)
                return mode.HasFlag(TangentMode.InFree);
            else
                return mode.HasFlag(TangentMode.OutFree);
        }

        /// <summary>
        /// Rebuilds the internal GUI elements. Should be called whenever timeline properties change.
        /// </summary>
        public override void Rebuild()
        {
            canvas.Clear();

            if (curveInfos == null)
                return;

            tickHandler.SetRange(rangeOffset, rangeOffset + GetRange(true), drawableWidth + PADDING);

            // Draw vertical frame markers
            int numTickLevels = tickHandler.NumLevels;
            for (int i = numTickLevels - 1; i >= 0; i--)
            {
                float[] ticks = tickHandler.GetTicks(i);
                float strength = tickHandler.GetLevelStrength(i);

                for (int j = 0; j < ticks.Length; j++)
                {
                    Color color = COLOR_DARK_GRAY;
                    color.a *= strength;

                    DrawFrameMarker(ticks[j], color, false);
                }
            }

            // Draw center line
            DrawCenterLine();

            // Draw curves
            int curveIdx = 0;
            foreach (var curveInfo in curveInfos)
            {
                DrawCurve(curveInfo.curve, curveInfo.color);

                // Draw keyframes
                KeyFrame[] keyframes = curveInfo.curve.KeyFrames;

                for (int i = 0; i < keyframes.Length; i++)
                {
                    bool isSelected = IsSelected(curveIdx, i);

                    DrawKeyframe(keyframes[i].time, keyframes[i].value, isSelected);

                    if (isSelected)
                        DrawTangents(keyframes[i], curveInfo.curve.TangentModes[i]);
                }

                curveIdx++;
            }

            // Draw selected frame marker
            if (markedFrameIdx != -1)
                DrawFrameMarker(GetTimeForFrame(markedFrameIdx), Color.BansheeOrange, true);
        }

        /// <summary>
        /// Checks is the provided key-frame currently marked as selected.
        /// </summary>
        /// <param name="curveIdx">Index of the curve the keyframe is on.</param>
        /// <param name="keyIdx">Index of the keyframe.</param>
        /// <returns>True if selected, false otherwise.</returns>
        private bool IsSelected(int curveIdx, int keyIdx)
        {
            if (selectedKeyframes == null)
                return false;

            if (curveIdx < 0 || curveIdx >= selectedKeyframes.Length)
                return false;

            if (keyIdx < 0 || keyIdx >= selectedKeyframes[curveIdx].Length)
                return false;

            return selectedKeyframes[curveIdx][keyIdx];
        }

        /// <summary>
        /// Draws the curve using the provided color.
        /// </summary>
        /// <param name="curve">Curve to draw within the currently set range. </param>
        /// <param name="color">Color to draw the curve with.</param>
        private void DrawCurve(EdAnimationCurve curve, Color color)
        {
            float range = GetRange();
            float lengthPerPixel = range / drawableWidth;

            KeyFrame[] keyframes = curve.KeyFrames;
            if (keyframes.Length <= 0)
                return;

            // Draw start line
            {
                float curveStart = MathEx.Clamp(keyframes[0].time, 0.0f, range);
                float curveValue = curve.Evaluate(0.0f, false);

                Vector2I start = CurveToPixelSpace(new Vector2(0.0f, curveValue));
                start.x -= GUIGraphTime.PADDING;

                Vector2I end = CurveToPixelSpace(new Vector2(curveStart, curveValue));

                canvas.DrawLine(start, end, COLOR_MID_GRAY);
            }

            List<Vector2I> linePoints = new List<Vector2I>();

            // Draw in between keyframes
            for (int i = 0; i < keyframes.Length - 1; i++)
            {
                float start = MathEx.Clamp(keyframes[i].time, 0.0f, range);
                float end = MathEx.Clamp(keyframes[i + 1].time, 0.0f, range);

                bool isStep = keyframes[i].outTangent == float.PositiveInfinity ||
                              keyframes[i + 1].inTangent == float.PositiveInfinity;

                // If step tangent, draw the required lines without sampling, as the sampling will miss the step
                if (isStep)
                {
                    float startValue = curve.Evaluate(start, false);
                    float endValue = curve.Evaluate(end, false);

                    linePoints.Add(CurveToPixelSpace(new Vector2(start, startValue)));
                    linePoints.Add(CurveToPixelSpace(new Vector2(end, startValue)));
                    linePoints.Add(CurveToPixelSpace(new Vector2(end, endValue)));
                }
                else // Draw normally
                {
                    float timeIncrement = LINE_SPLIT_WIDTH*lengthPerPixel;

                    int startPixel = (int)(start / lengthPerPixel);
                    int endPixel = (int)(end / lengthPerPixel);

                    int numSplits;
                    if (startPixel != endPixel)
                    {
                        float fNumSplits = (end - start) / timeIncrement;

                        numSplits = MathEx.FloorToInt(fNumSplits);
                        float remainder = fNumSplits - numSplits;

                        float lengthRounded = (end - start)*(numSplits/fNumSplits);
                        timeIncrement = lengthRounded/numSplits;

                        numSplits += MathEx.CeilToInt(remainder) + 1;
                    }
                    else
                    {
                        numSplits = 1;
                        timeIncrement = 0.0f;
                    }

                    for (int j = 0; j < numSplits; j++)
                    {
                        float t = Math.Min(start + j * timeIncrement, end);
                        float value = curve.Evaluate(t, false);

                        linePoints.Add(CurveToPixelSpace(new Vector2(t, value)));
                    }
                }
            }
            
            canvas.DrawPolyLine(linePoints.ToArray(), color);

            // Draw end line
            {
                float curveEnd = MathEx.Clamp(keyframes[keyframes.Length - 1].time, 0.0f, range);
                float curveValue = curve.Evaluate(range, false);

                Vector2I start = CurveToPixelSpace(new Vector2(curveEnd, curveValue));
                Vector2I end = new Vector2I(width, start.y);

                canvas.DrawLine(start, end, COLOR_MID_GRAY);
            }
        }
    }

    /// <summary>
    /// Information necessary to draw a curve.
    /// </summary>
    internal struct CurveDrawInfo
    {
        public CurveDrawInfo(EdAnimationCurve curve, Color color)
        {
            this.curve = curve;
            this.color = color;
        }

        public EdAnimationCurve curve;
        public Color color;
    }

    /** }@ */
}

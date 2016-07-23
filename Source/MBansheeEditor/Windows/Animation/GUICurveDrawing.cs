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
    internal class GUICurveDrawing
    {
        private const int LINE_SPLIT_WIDTH = 2;
        private const int TANGENT_LINE_DISTANCE = 30;
        private static readonly Color COLOR_MID_GRAY = new Color(90.0f / 255.0f, 90.0f / 255.0f, 90.0f / 255.0f, 1.0f);
        private static readonly Color COLOR_DARK_GRAY = new Color(40.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 1.0f);

        private EdAnimationCurve[] curves;
        private bool[][] selectedKeyframes;

        private int width;
        private int height;
        private float xRange = 60.0f;
        private float yRange = 20.0f;
        private int fps = 1;
        private int markedFrameIdx = 0;

        private int drawableWidth;
        private GUICanvas canvas;
        private GUIGraphTicks tickHandler;

        /// <summary>
        /// Creates a new curve drawing GUI element.
        /// </summary>
        /// <param name="layout">Layout into which to add the GUI element.</param>
        /// <param name="width">Width of the element in pixels.</param>
        /// <param name="height">Height of the element in pixels.</param>
        /// <param name="curves">Initial set of curves to display. </param>
        public GUICurveDrawing(GUILayout layout, int width, int height, EdAnimationCurve[] curves)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            tickHandler = new GUIGraphTicks(GUITickStepType.Time);

            this.curves = curves;
            
            SetSize(width, height);
            ClearSelectedKeyframes(); // Makes sure the array is initialized

            Rebuild();
        }

        /// <summary>
        /// Change the set of curves to display.
        /// </summary>
        /// <param name="curves">New set of curves to draw on the GUI element.</param>
        public void SetCurves(EdAnimationCurve[] curves)
        {
            this.curves = curves;
        }

        /// <summary>
        /// Change the physical size of the GUI element.
        /// </summary>
        /// <param name="width">Width of the element in pixels.</param>
        /// <param name="height">Height of the element in pixels.</param>
        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            drawableWidth = Math.Max(0, width - GUIGraphTime.PADDING * 2);
        }

        /// <summary>
        /// Changes the visible range that the GUI element displays.
        /// </summary>
        /// <param name="xRange">Range of the horizontal area. Displayed area will range from [0, xRange].</param>
        /// <param name="yRange">Range of the vertical area. Displayed area will range from 
        ///                      [-yRange * 0.5, yRange * 0.5]</param>
        public void SetRange(float xRange, float yRange)
        {
            this.xRange = xRange;
            this.yRange = yRange;
        }

        /// <summary>
        /// Number of frames per second, used for frame selection and marking.
        /// </summary>
        /// <param name="fps">Number of prames per second.</param>
        public void SetFPS(int fps)
        {
            this.fps = Math.Max(1, fps);
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
            selectedKeyframes = new bool[curves.Length][];

            for (int i = 0; i < curves.Length; i++)
            {
                KeyFrame[] keyframes = curves[i].KeyFrames;
                selectedKeyframes[i] = new bool[keyframes.Length];
            }
        }

        /// <summary>
        /// Returns time for a frame with the specified index. Depends on set range and FPS.
        /// </summary>
        /// <param name="frameIdx">Index of the frame (not a key-frame) to get the time for.</param>
        /// <returns>Time of the frame with the provided index. </returns>
        public float GetTimeForFrame(int frameIdx)
        {
            float range = GetRange();
            int numFrames = (int)range * fps;
            float timePerFrame = range / numFrames;

            return frameIdx* timePerFrame;
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
            for (int i = 0; i < curves.Length; i++)
            {
                EdAnimationCurve curve = curves[i];
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
            for (int i = 0; i < curves.Length; i++)
            {
                EdAnimationCurve curve = curves[i];
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
        /// <returns>True if the window coordinates were within the curve area, false otherwise.</returns>
        public bool PixelToCurveSpace(Vector2I pixelCoords, out Vector2 curveCoords)
        {
            Rect2I bounds = canvas.Bounds;

            // Check if outside of curve drawing bounds
            if (pixelCoords.x < (bounds.x + GUIGraphTime.PADDING) || pixelCoords.x >= (bounds.x + bounds.width - GUIGraphTime.PADDING) ||
                pixelCoords.y < bounds.y || pixelCoords.y >= (bounds.y + bounds.height))
            {
                curveCoords = new Vector2();
                return false;
            }

            // Find time and value of the place under the coordinates
            Vector2I relativeCoords = pixelCoords - new Vector2I(bounds.x + GUIGraphTime.PADDING, bounds.y);

            float lengthPerPixel = GetRange() / drawableWidth;
            float heightPerPixel = yRange / height;

            float yOffset = yRange / 2.0f;

            float t = relativeCoords.x * lengthPerPixel;
            float value = yOffset - relativeCoords.y * heightPerPixel;

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

            Vector2I pixelCoords = new Vector2I();
            pixelCoords.x = (int)((curveCoords.x / GetRange()) * drawableWidth) + GUIGraphTime.PADDING;
            pixelCoords.y = heightOffset - (int)((curveCoords.y / yRange) * height);

            return pixelCoords;
        }

        /// <summary>
        /// Draws a vertical frame marker on the curve area.
        /// </summary>
        /// <param name="t">Time at which to draw the marker.</param>
        /// <param name="color">Color with which to draw the marker.</param>
        private void DrawFrameMarker(float t, Color color)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + GUIGraphTime.PADDING;

            Vector2I start = new Vector2I(xPos, 0);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, color);
        }

        /// <summary>
        /// Draws a horizontal line representing the line at y = 0.
        /// </summary>
        private void DrawCenterLine()
        {
            int heightOffset = height / 2; // So that y = 0 is at center of canvas

            Vector2I start = new Vector2I(0, heightOffset);
            Vector2I end = new Vector2I(width, heightOffset);

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
                return !mode.HasFlag(TangentMode.InAuto);
            else
                return !mode.HasFlag(TangentMode.OutAuto);
        }

        /// <summary>
        /// Returns the range of times displayed by the timeline rounded to the multiple of FPS.
        /// </summary>
        /// <param name="padding">If true, extra range will be included to cover the right-most padding.</param>
        /// <returns>Time range rounded to a multiple of FPS.</returns>
        private float GetRange(bool padding = false)
        {
            float spf = 1.0f / fps;

            float range = xRange;
            if (padding)
            {
                float lengthPerPixel = xRange / drawableWidth;
                range += lengthPerPixel * GUIGraphTime.PADDING;
            }

            return ((int)range / spf) * spf;
        }

        /// <summary>
        /// Rebuilds the internal GUI elements. Should be called whenever timeline properties change.
        /// </summary>
        public void Rebuild()
        {
            canvas.Clear();

            if (curves == null)
                return;

            tickHandler.SetRange(0.0f, GetRange(true), drawableWidth + GUIGraphTime.PADDING);

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

                    DrawFrameMarker(ticks[j], color);
                }
            }

            // Draw center line
            DrawCenterLine();

            // Draw curves
            int curveIdx = 0;
            foreach (var curve in curves)
            {
                Color color = GetUniqueColor(curveIdx);
                DrawCurve(curve, color);

                // Draw keyframes
                KeyFrame[] keyframes = curve.KeyFrames;

                for (int i = 0; i < keyframes.Length; i++)
                {
                    bool isSelected = IsSelected(curveIdx, i);

                    DrawKeyframe(keyframes[i].time, keyframes[i].value, isSelected);

                    if (isSelected)
                        DrawTangents(keyframes[i], curve.TangentModes[i]);
                }

                curveIdx++;
            }

            // Draw selected frame marker
            if (markedFrameIdx != -1)
                DrawFrameMarker(GetTimeForFrame(markedFrameIdx), Color.BansheeOrange);
        }

        /// <summary>
        /// Generates a unique color based on the provided index.
        /// </summary>
        /// <param name="idx">Index to use for generating a color. Should be less than 30 in order to guarantee reasonably
        /// different colors.</param>
        /// <returns>Unique color.</returns>
        private Color GetUniqueColor(int idx)
        {
            const int COLOR_SPACING = 359 / 15;

            float hue = ((idx * COLOR_SPACING) % 359) / 359.0f;
            return Color.HSV2RGB(new Color(hue, 175.0f / 255.0f, 175.0f / 255.0f));
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
            float pixelsPerHeight = height/yRange;

            int heightOffset = height/2; // So that y = 0 is at center of canvas

            KeyFrame[] keyframes = curve.KeyFrames;
            if (keyframes.Length < 0)
                return;

            // Draw start line
            {
                float start = MathEx.Clamp(keyframes[0].time, 0.0f, range);
                int startPixel = (int)(start / lengthPerPixel);

                int xPosStart = 0;
                int xPosEnd = GUIGraphTime.PADDING + startPixel;

                int yPos = (int)(curve.Evaluate(0.0f, false) * pixelsPerHeight);
                yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                Vector2I a = new Vector2I(xPosStart, yPos);
                Vector2I b = new Vector2I(xPosEnd, yPos);

                canvas.DrawLine(a, b, COLOR_MID_GRAY);
            }

            List<Vector2I> linePoints = new List<Vector2I>();

            // Draw in between keyframes
            for (int i = 0; i < keyframes.Length - 1; i++)
            {
                float start = MathEx.Clamp(keyframes[i].time, 0.0f, range);
                float end = MathEx.Clamp(keyframes[i + 1].time, 0.0f, range);
                
                int startPixel = (int)(start / lengthPerPixel);
                int endPixel = (int)(end / lengthPerPixel);

                bool isStep = keyframes[i].outTangent == float.PositiveInfinity ||
                              keyframes[i + 1].inTangent == float.PositiveInfinity;

                // If step tangent, draw the required lines without sampling, as the sampling will miss the step
                if (isStep)
                {
                    // Line from left to right frame
                    int xPos = startPixel;
                    int yPosStart = (int)(curve.Evaluate(start, false) * pixelsPerHeight);
                    yPosStart = heightOffset - yPosStart; // Offset and flip height (canvas Y goes down)

                    linePoints.Add(new Vector2I(GUIGraphTime.PADDING + xPos, yPosStart));

                    xPos = endPixel;
                    linePoints.Add(new Vector2I(GUIGraphTime.PADDING + xPos, yPosStart));

                    // Line representing the step
                    int yPosEnd = (int)(curve.Evaluate(end, false) * pixelsPerHeight);
                    yPosEnd = heightOffset - yPosEnd; // Offset and flip height (canvas Y goes down)

                    linePoints.Add(new Vector2I(GUIGraphTime.PADDING + xPos, yPosEnd));
                }
                else // Draw normally
                {
                    int numSplits;
                    float timeIncrement;
                    if (startPixel != endPixel)
                    {
                        float fNumSplits = (endPixel - startPixel)/(float) LINE_SPLIT_WIDTH;

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
                        int xPos = Math.Min(startPixel + j * LINE_SPLIT_WIDTH, endPixel);
                        float t = Math.Min(start + j * timeIncrement, end);

                        int yPos = (int)(curve.Evaluate(t, false) * pixelsPerHeight);
                        yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                        linePoints.Add(new Vector2I(GUIGraphTime.PADDING + xPos, yPos));
                    }
                }
            }
            
            canvas.DrawPolyLine(linePoints.ToArray(), color);

            // Draw end line
            {
                float end = MathEx.Clamp(keyframes[keyframes.Length - 1].time, 0.0f, range);
                int endPixel = (int)(end / lengthPerPixel);

                int xPosStart = GUIGraphTime.PADDING + endPixel;
                int xPosEnd = width;

                int yPos = (int)(curve.Evaluate(range, false) * pixelsPerHeight);
                yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                Vector2I a = new Vector2I(xPosStart, yPos);
                Vector2I b = new Vector2I(xPosEnd, yPos);

                canvas.DrawLine(a, b, COLOR_MID_GRAY);
            }
        }
    }

    /** }@ */
}

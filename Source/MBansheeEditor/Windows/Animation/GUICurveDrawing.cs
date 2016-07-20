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

    // TODO DOC
    internal class GUICurveDrawing
    {
        private const int LINE_SPLIT_WIDTH = 2;
        private static readonly Color COLOR_MID_GRAY = new Color(90.0f / 255.0f, 90.0f / 255.0f, 90.0f / 255.0f, 1.0f);
        private static readonly Color COLOR_DARK_GRAY = new Color(40.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 1.0f);

        private EdAnimationCurve[] curves;
        private int width;
        private int height;
        private float xRange = 60.0f;
        private float yRange = 20.0f;
        private int fps = 1;
        private int markedFrameIdx = -1;

        private int drawableWidth;
        private GUICanvas canvas;
        private GUITicks tickHandler;

        public GUICurveDrawing(GUILayout layout, int width, int height, EdAnimationCurve[] curves)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            tickHandler = new GUITicks(GUITickStepType.Time);

            this.curves = curves;

            SetSize(width, height);
        }

        public void SetCurves(EdAnimationCurve[] curves)
        {
            this.curves = curves;

            Rebuild();
        }

        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            drawableWidth = Math.Max(0, width - GUITimeline.PADDING * 2);

            tickHandler.SetRange(0.0f, xRange, drawableWidth);

            Rebuild();
        }

        public void SetRange(float xRange, float yRange)
        {
            this.xRange = xRange;
            this.yRange = yRange;

            tickHandler.SetRange(0.0f, GetRange(), drawableWidth);

            Rebuild();
        }

        public void SetFPS(int fps)
        {
            this.fps = Math.Max(1, fps);

            tickHandler.SetRange(0.0f, GetRange(), drawableWidth);

            Rebuild();
        }

        // Set to -1 to clear it
        public void SetMarkedFrame(int frameIdx)
        {
            markedFrameIdx = frameIdx;

            Rebuild();
        }

        public bool GetCurveCoordinates(Vector2I windowCoords, out Vector2 curveCoords)
        {
            Rect2I bounds = canvas.Bounds;

            if (windowCoords.x < (bounds.x + GUITimeline.PADDING) || windowCoords.x >= (bounds.x + bounds.width - GUITimeline.PADDING) ||
                windowCoords.y < bounds.y || windowCoords.y >= (bounds.y + bounds.height))
            {
                curveCoords = new Vector2();
                return false;
            }

            Vector2I relativeCoords = windowCoords - new Vector2I(bounds.x + GUITimeline.PADDING, bounds.y);

            float lengthPerPixel = xRange / drawableWidth;
            float heightPerPixel = yRange / height;

            float yOffset = yRange/2.0f;

            curveCoords = new Vector2();
            curveCoords.x = relativeCoords.x * lengthPerPixel;
            curveCoords.y = yOffset - relativeCoords.y * heightPerPixel;

            return true;
        }

        private void DrawFrameMarker(float t, Color color)
        {
            int xPos = (int)((t / GetRange()) * drawableWidth) + GUITimeline.PADDING;

            Vector2I start = new Vector2I(xPos, 0);
            Vector2I end = new Vector2I(xPos, height);

            canvas.DrawLine(start, end, color);
        }

        private void DrawCenterLine()
        {
            int heightOffset = height / 2; // So that y = 0 is at center of canvas

            Vector2I start = new Vector2I(0, heightOffset);
            Vector2I end = new Vector2I(width, heightOffset);

            canvas.DrawLine(start, end, COLOR_DARK_GRAY);
        }

        // Returns range rounded to the nearest multiple of FPS
        private float GetRange()
        {
            float spf = 1.0f / fps;

            return ((int)xRange / spf) * spf;
        }

        private void Rebuild()
        {
            canvas.Clear();

            if (curves == null)
                return;

            // Draw vertical frame markers
            int numTickLevels = tickHandler.NumLevels;
            for (int i = numTickLevels - 1; i >= 0; i--)
            {
                float[] ticks = tickHandler.GetTicks(i);
                float strength = tickHandler.GetLevelStrength(i);

                for (int j = 0; j < ticks.Length; j++)
                {
                    Color color = COLOR_DARK_GRAY;
                    color.a *= MathEx.Clamp01(strength);

                    DrawFrameMarker(ticks[j], color);
                }
            }

            // Draw center line
            DrawCenterLine();

            // Draw curves
            int idx = 0;
            foreach (var curve in curves)
            {
                Color color = GetUniqueColor(idx);
                DrawCurve(curve, color);
                idx++;
            }

            // Draw selected frame marker
            if (markedFrameIdx != -1)
            {
                float range = GetRange();
                int numFrames = (int)range * fps;
                float timePerFrame = range / numFrames;

                DrawFrameMarker(markedFrameIdx*timePerFrame, Color.BansheeOrange);
            }
        }

        private Color GetUniqueColor(int idx)
        {
            const int COLOR_SPACING = 359 / 15;

            float hue = ((idx * COLOR_SPACING) % 359) / 359.0f;
            return Color.HSV2RGB(new Color(hue, 175.0f / 255.0f, 175.0f / 255.0f));
        }

        private void DrawCurve(EdAnimationCurve curve, Color color)
        {
            float lengthPerPixel = xRange/drawableWidth;
            float pixelsPerHeight = height/yRange;

            int heightOffset = height/2; // So that y = 0 is at center of canvas

            KeyFrame[] keyframes = curve.Native.KeyFrames;
            if (keyframes.Length < 0)
                return;

            // Draw start line
            {
                float start = MathEx.Clamp(keyframes[0].time, 0.0f, xRange);
                int startPixel = (int)(start / lengthPerPixel);

                int xPosStart = 0;
                int xPosEnd = GUITimeline.PADDING + startPixel;

                int yPos = (int)(curve.Native.Evaluate(0.0f, false) * pixelsPerHeight);
                yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                Vector2I a = new Vector2I(xPosStart, yPos);
                Vector2I b = new Vector2I(xPosEnd, yPos);

                canvas.DrawLine(a, b, COLOR_MID_GRAY);
            }

            List<Vector2I> linePoints = new List<Vector2I>();

            // Draw in between keyframes
            for (int i = 0; i < keyframes.Length - 1; i++)
            {
                float start = MathEx.Clamp(keyframes[i].time, 0.0f, xRange);
                float end = MathEx.Clamp(keyframes[i + 1].time, 0.0f, xRange);
                
                int startPixel = (int)(start / lengthPerPixel);
                int endPixel = (int)(end / lengthPerPixel);

                bool isStep = keyframes[i].outTangent == float.PositiveInfinity ||
                              keyframes[i + 1].inTangent == float.PositiveInfinity;

                // If step tangent, draw the required lines without sampling, as the sampling will miss the step
                if (isStep)
                {
                    // Line from left to right frame
                    int xPos = startPixel;
                    int yPosStart = (int)(curve.Native.Evaluate(start, false) * pixelsPerHeight);
                    yPosStart = heightOffset - yPosStart; // Offset and flip height (canvas Y goes down)

                    linePoints.Add(new Vector2I(GUITimeline.PADDING + xPos, yPosStart));

                    xPos = endPixel;
                    linePoints.Add(new Vector2I(GUITimeline.PADDING + xPos, yPosStart));

                    // Line representing the step
                    int yPosEnd = (int)(curve.Native.Evaluate(end, false) * pixelsPerHeight);
                    yPosEnd = heightOffset - yPosEnd; // Offset and flip height (canvas Y goes down)

                    linePoints.Add(new Vector2I(GUITimeline.PADDING + xPos, yPosEnd));
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

                        int yPos = (int)(curve.Native.Evaluate(t, false) * pixelsPerHeight);
                        yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                        linePoints.Add(new Vector2I(GUITimeline.PADDING + xPos, yPos));
                    }
                }
            }
            
            canvas.DrawPolyLine(linePoints.ToArray(), color);

            // Draw end line
            {
                float end = MathEx.Clamp(keyframes[keyframes.Length - 1].time, 0.0f, xRange);
                int endPixel = (int)(end / lengthPerPixel);

                int xPosStart = GUITimeline.PADDING + endPixel;
                int xPosEnd = width;

                int yPos = (int)(curve.Native.Evaluate(xRange, false) * pixelsPerHeight);
                yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                Vector2I a = new Vector2I(xPosStart, yPos);
                Vector2I b = new Vector2I(xPosEnd, yPos);

                canvas.DrawLine(a, b, COLOR_MID_GRAY);
            }
        }
    }

    /** }@ */
}

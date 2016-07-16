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
        private const int PADDING = 30; // TODO - Shared with GUITimeline
        private const int LINE_SPLIT_WIDTH = 2;
        private static readonly Color COLOR_MID_GRAY = new Color(90.0f / 255.0f, 90.0f / 255.0f, 90.0f / 255.0f, 1.0f);

        private EdAnimationCurve[] curves;
        private int width;
        private int height;
        private float xRange = 60.0f;
        private float yRange = 20.0f;

        private int drawableWidth;
        private GUICanvas canvas;

        public GUICurveDrawing(GUILayout layout, int width, int height, EdAnimationCurve[] curves)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            this.curves = curves;

            SetSize(width, height);
        }

        public bool GetCurveCoordinates(Vector2I windowCoords, out Vector2 curveCoords)
        {
            Rect2I bounds = canvas.Bounds;

            if (windowCoords.x < (bounds.x + PADDING) || windowCoords.x >= (bounds.x + bounds.width - PADDING) ||
                windowCoords.y < bounds.y || windowCoords.y >= (bounds.y + bounds.height))
            {
                curveCoords = new Vector2();
                return false;
            }

            Vector2I relativeCoords = windowCoords - new Vector2I(bounds.x + PADDING, bounds.y);

            float lengthPerPixel = xRange / drawableWidth;
            float heightPerPixel = yRange / height;

            float yOffset = yRange/2.0f;

            curveCoords = new Vector2();
            curveCoords.x = relativeCoords.x * lengthPerPixel;
            curveCoords.y = yOffset - relativeCoords.y * heightPerPixel;

            return true;
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

            drawableWidth = Math.Max(0, width - PADDING * 2);

            Rebuild();
        }

        public void SetRange(float xRange, float yRange)
        {
            this.xRange = xRange;
            this.yRange = yRange;

            Rebuild();
        }

        private void Rebuild()
        {
            canvas.Clear();

            if (curves == null)
                return;

            // TODO - Draw vertical frame markers
            // TODO - Draw horizontal value markers with text

            foreach (var curve in curves)
            {
                // TODO - Pick unique color for each curve
                DrawCurve(curve, Color.Red);
            }
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
                int xPosEnd = PADDING + startPixel;

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

                // TODO - Make sure that step tangents work

                for (int j = 0; j < numSplits; j++)
                {
                    int xPos = Math.Min(startPixel + j * LINE_SPLIT_WIDTH, endPixel);
                    float t = Math.Min(start + j * timeIncrement, end);

                    int yPos = (int)(curve.Native.Evaluate(t, false) * pixelsPerHeight);
                    yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                    linePoints.Add(new Vector2I(PADDING + xPos, yPos));
                }
            }
            
            canvas.DrawPolyLine(linePoints.ToArray(), color);

            // Draw end line
            {
                float end = MathEx.Clamp(keyframes[keyframes.Length - 1].time, 0.0f, xRange);
                int endPixel = (int)(end / lengthPerPixel);

                int xPosStart = PADDING + endPixel;
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

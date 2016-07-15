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
        private const int LINE_SPLIT_WIDTH = 5;

        private EdAnimationCurve[] curves;
        private int width;
        private int height;
        private float length = 60.0f;
        private float minY = -1.0f;
        private float maxY = 1.0f;

        private int drawableWidth;
        private GUICanvas canvas;

        public GUICurveDrawing(GUILayout layout, int width, int height, EdAnimationCurve[] curves)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

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

            drawableWidth = Math.Max(0, width - PADDING * 2);

            Rebuild();
        }

        public void SetRange(float length, float minY, float maxY)
        {
            this.length = length;
            this.minY = minY;
            this.maxY = maxY;

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
            float lengthPerPixel = length/drawableWidth;
            float heightPerPixel = (maxY - minY)/height;

            int heightOffset = height/2; // So that y = 0 is at center of canvas

            KeyFrame[] keyframes = curve.Native.KeyFrames;
            if (keyframes.Length == 0)
                return;

            float start = MathEx.Clamp(keyframes[0].time, 0.0f, length);
            float end = MathEx.Clamp(keyframes[keyframes.Length - 1].time, 0.0f, length);

            int startPixel = (int)(start * lengthPerPixel);
            int endPixel = (int)(end * lengthPerPixel);

            // Draw start line
            {
                int xPosStart = -PADDING;
                int xPosEnd = startPixel;

                int yPos = (int)(curve.Native.Evaluate(0.0f, false) * heightPerPixel);
                yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                Vector2I a = new Vector2I(xPosStart, yPos);
                Vector2I b = new Vector2I(xPosEnd, yPos);

                canvas.DrawLine(a, b, Color.LightGray);
            }

            // Draw in between
            List<Vector2I> linePoints = new List<Vector2I>();

            int xPos = startPixel;
            do
            {
                float t = xPos * lengthPerPixel;

                int yPos = (int)(curve.Native.Evaluate(t, false) * heightPerPixel);
                yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                linePoints.Add(new Vector2I(xPos, yPos));

                xPos += LINE_SPLIT_WIDTH;
            } while (xPos <= endPixel);

            canvas.DrawPolyLine(linePoints.ToArray(), color);

            // Draw end line
            {
                int xPosStart = endPixel;
                int xPosEnd = width;

                int yPos = (int)(curve.Native.Evaluate(length, false) * heightPerPixel);
                yPos = heightOffset - yPos; // Offset and flip height (canvas Y goes down)

                Vector2I a = new Vector2I(xPosStart, yPos);
                Vector2I b = new Vector2I(xPosEnd, yPos);

                canvas.DrawLine(a, b, Color.LightGray);
            }
        }
    }

    /** }@ */
}

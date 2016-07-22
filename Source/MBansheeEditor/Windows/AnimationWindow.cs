//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// Displays animation curve editor window.
    /// </summary>
    internal class AnimationWindow : EditorWindow
    {
        private GUIFloatField lengthField;
        private GUIIntField fpsField;
        private GUIFloatField yRangeField;
        private GUIButton addKeyframeBtn;

        private GUILayout buttonLayout;
        private int buttonLayoutHeight;

        private GUIPanel editorPanel;
        private GUICurveEditor guiCurveEditor;

        /// <summary>
        /// Opens the animation window.
        /// </summary>
        [MenuItem("Windows/Animation", ButtonModifier.CtrlAlt, ButtonCode.A, 6000)]
        private static void OpenGameWindow()
        {
            OpenWindow<AnimationWindow>();
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Animation");
        }

        private void OnInitialize()
        {
            lengthField = new GUIFloatField(new LocEdString("Length"), 50);
            fpsField = new GUIIntField(new LocEdString("FPS"), 50);
            yRangeField = new GUIFloatField(new LocEdString("Y range"), 50);
            addKeyframeBtn = new GUIButton(new LocEdString("Add keyframe"));

            lengthField.Value = 60.0f;
            fpsField.Value = 1;
            yRangeField.Value = 20.0f;

            lengthField.OnChanged += x =>
            {
                guiCurveEditor.SetRange(lengthField.Value, yRangeField.Value);
            };
            fpsField.OnChanged += x =>
            {
                guiCurveEditor.SetFPS(x);
            };
            yRangeField.OnChanged += x =>
            {
                guiCurveEditor.SetRange(lengthField.Value, yRangeField.Value);
            };
            addKeyframeBtn.OnClick += () =>
            {
                guiCurveEditor.AddKeyFrameAtMarker();
            };

            GUILayout mainLayout = GUI.AddLayoutY();

            buttonLayout = mainLayout.AddLayoutX();
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(lengthField);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(yRangeField);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(fpsField);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(addKeyframeBtn);
            buttonLayout.AddSpace(5);

            editorPanel = mainLayout.AddPanel();
            buttonLayoutHeight = lengthField.Bounds.height;

            guiCurveEditor = new GUICurveEditor(editorPanel, Width, Height - buttonLayoutHeight);
            guiCurveEditor.SetCurves(CreateDummyCurves());
            guiCurveEditor.Redraw();
        }

        private EdAnimationCurve[] CreateDummyCurves()
        {
            EdAnimationCurve[] curves = new EdAnimationCurve[1];
            curves[0] = new EdAnimationCurve();

            curves[0].AddKeyframe(0.0f, 1.0f);
            curves[0].AddKeyframe(10.0f, 5.0f);
            curves[0].AddKeyframe(15.0f, -2.0f);
            curves[0].AddKeyframe(20.0f, 3.0f, TangentMode.InStep);

            return curves;
        }

        protected override void WindowResized(int width, int height)
        {
            guiCurveEditor.SetSize(width, height - buttonLayoutHeight);
            guiCurveEditor.Redraw();
        }

        private void OnEditorUpdate()
        {
            Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);
            Rect2I curveEditorBounds = editorPanel.Bounds;

            Vector2I offset = new Vector2I(curveEditorBounds.x, curveEditorBounds.y);
            guiCurveEditor.HandleInput(windowPos - offset);
        }
    }

    /** @} */
}

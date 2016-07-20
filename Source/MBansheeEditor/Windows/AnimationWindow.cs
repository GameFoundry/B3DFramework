//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
        private GUIGraphTime timeline;
        private GUICurveDrawing curveDrawing;
        private GUIGraphValues sidebar;

        private GUIFloatField lengthField;
        private GUIIntField fpsField;
        private GUIFloatField yRangeField;

        private GUILayout buttonLayout;

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

            lengthField.Value = 60.0f;
            fpsField.Value = 1;
            yRangeField.Value = 20.0f;

            lengthField.OnChanged += x =>
            {
                timeline.SetRange(lengthField.Value);
                curveDrawing.SetRange(lengthField.Value, yRangeField.Value);
            };
            fpsField.OnChanged += x =>
            {
                timeline.SetFPS(x);
                curveDrawing.SetFPS(x);
            };
            yRangeField.OnChanged += x =>
            {
                curveDrawing.SetRange(lengthField.Value, x);
                sidebar.SetRange(x * -0.5f, x * 0.5f);
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

            timeline = new GUIGraphTime(mainLayout, Width, 20);

            EdAnimationCurve[] curves = CreateDummyCurves();
            curveDrawing = new GUICurveDrawing(mainLayout, Width, Height - 20, curves);
            curveDrawing.SetRange(60.0f, 20.0f);

            GUIPanel sidebarPanel = GUI.AddPanel(-10);
            sidebarPanel.SetPosition(0, 20 + buttonLayout.Bounds.height);

            sidebar = new GUIGraphValues(sidebarPanel, 30, Height - 20 - buttonLayout.Bounds.height);

            curveDrawing.SetSize(Width, Height - 20 - buttonLayout.Bounds.height);

            // TODO - Calculate min/max Y and range to set as default
            //  - Also recalculate whenever curves change and increase as needed
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
            timeline.SetSize(width, 20);
            curveDrawing.SetSize(width, height - 20 - buttonLayout.Bounds.height);
            sidebar.SetSize(30, height - 20 - buttonLayout.Bounds.height);
        }

        private void OnEditorUpdate()
        {
            if (Input.IsPointerButtonHeld(PointerButton.Left))
            {
                Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);

                Vector2 curveCoord;
                if (curveDrawing.GetCurveCoordinates(windowPos, out curveCoord))
                {
                    Debug.Log("Click coord: " + curveCoord);
                }
                else
                {
                    int frameIdx = timeline.GetFrame(windowPos);
                    timeline.SetMarkedFrame(frameIdx);
                    curveDrawing.SetMarkedFrame(frameIdx);
                }
            }
        }
    }

    /** @} */
}

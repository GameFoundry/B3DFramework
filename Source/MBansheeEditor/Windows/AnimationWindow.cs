//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

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
        public struct KeyframeRef
        {
            public KeyframeRef(int curveIdx, int keyIdx)
            {
                this.curveIdx = curveIdx;
                this.keyIdx = keyIdx;
            }

            public int curveIdx;
            public int keyIdx;
        }

        private GUIGraphTime timeline;
        private GUICurveDrawing curveDrawing;
        private GUIGraphValues sidebar;

        private GUIFloatField lengthField;
        private GUIIntField fpsField;
        private GUIFloatField yRangeField;
        private GUIButton addKeyframeBtn;

        private GUILayout buttonLayout;

        private EdAnimationCurve[] curves = new EdAnimationCurve[0];
        private int markedFrameIdx;
        private List<KeyframeRef> selectedKeyframes = new List<KeyframeRef>();

        // Keyframe drag
        private bool isMousePressedOverKey;
        private KeyFrame[] draggedKeyframes;
        private Vector2 dragStart;

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
            addKeyframeBtn.OnClick += () =>
            {
                AddKeyframeAtMarker();
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

            timeline = new GUIGraphTime(mainLayout, Width, 20);

            curves = CreateDummyCurves();
            curveDrawing = new GUICurveDrawing(mainLayout, Width, Height - 20, curves);
            curveDrawing.SetRange(60.0f, 20.0f);

            GUIPanel sidebarPanel = GUI.AddPanel(-10);
            sidebarPanel.SetPosition(0, 20 + buttonLayout.Bounds.height);

            sidebar = new GUIGraphValues(sidebarPanel, 30, Height - 20 - buttonLayout.Bounds.height);
            sidebar.SetRange(-10.0f, 10.0f);

            curveDrawing.SetSize(Width, Height - 20 - buttonLayout.Bounds.height);
            curveDrawing.Rebuild();


            // TODO - Calculate min/max Y and range to set as default
            //  - Also recalculate whenever curves change and increase as needed
        }

        private void AddKeyframeAtMarker()
        {
            ClearSelection();

            foreach (var curve in curves)
            {
                float t = curveDrawing.GetTimeForFrame(markedFrameIdx);
                float value = curve.Native.Evaluate(t);

                curve.AddKeyframe(t, value);
            }

            curveDrawing.Rebuild();
        }

        private void DeleteSelectedKeyframes()
        {
            // Sort keys from highest to lowest so they can be removed without changing the indices of the keys
            // after them
            selectedKeyframes.Sort((x, y) =>
            {
                if (x.curveIdx.Equals(y.curveIdx))
                    return y.keyIdx.CompareTo(x.keyIdx);

                return x.curveIdx.CompareTo(y.curveIdx);
            });

            foreach (var keyframe in selectedKeyframes)
                curves[keyframe.curveIdx].RemoveKeyframe(keyframe.keyIdx);

            ClearSelection();

            curveDrawing.Rebuild();
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

            curveDrawing.Rebuild();
        }

        private void ClearSelection()
        {
            curveDrawing.ClearSelectedKeyframes();
            selectedKeyframes.Clear();
            isMousePressedOverKey = false;
        }

        private void OnEditorUpdate()
        {
            if (Input.IsPointerButtonDown(PointerButton.Left))
            {
                Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);

                Vector2 curveCoord;
                int curveIdx;
                int keyIdx;
                if (curveDrawing.GetCoordInfo(windowPos, out curveCoord, out curveIdx, out keyIdx))
                {
                    if (keyIdx == -1)
                        ClearSelection();
                    else
                    {
                        if (!Input.IsButtonHeld(ButtonCode.LeftShift) && !Input.IsButtonHeld(ButtonCode.RightShift))
                            ClearSelection();

                        curveDrawing.SelectKeyframe(curveIdx, keyIdx, true);

                        int existingIdx = selectedKeyframes.FindIndex(x =>
                        {
                            return x.curveIdx == curveIdx && x.keyIdx == keyIdx;
                        });

                        if (existingIdx == -1)
                            selectedKeyframes.Add(new KeyframeRef(curveIdx, keyIdx));

                        isMousePressedOverKey = true;
                        dragStart = curveCoord;
                    }

                    curveDrawing.Rebuild();
                }
            }
            else if (Input.IsPointerButtonHeld(PointerButton.Left))
            {
                Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);

                if (isMousePressedOverKey)
                {
                    // TODO - Check if pointer moves some minimal amount
                    // - If so, start drag. Record all current positions
                    // - Calculate offset in curve space and apply to all keyframes
                }
                else
                {
                    int frameIdx = timeline.GetFrame(windowPos);

                    if (frameIdx != -1)
                    {
                        timeline.SetMarkedFrame(frameIdx);
                        curveDrawing.SetMarkedFrame(frameIdx);

                        markedFrameIdx = frameIdx;
                        curveDrawing.Rebuild();
                    }
                }
            }
            else if (Input.IsPointerButtonUp(PointerButton.Left))
            {
                isMousePressedOverKey = false;
            }

            if(Input.IsButtonUp(ButtonCode.Delete))
                DeleteSelectedKeyframes();
        }
    }

    /** @} */
}

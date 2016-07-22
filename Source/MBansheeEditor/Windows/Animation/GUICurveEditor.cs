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

    internal class GUICurveEditor
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

        private const int TIMELINE_HEIGHT = 20;
        private const int SIDEBAR_WIDTH = 30;

        private GUILayout gui;
        private GUIPanel drawingPanel;
        private GUIGraphTime guiTimeline;
        private GUICurveDrawing guiCurveDrawing;
        private GUIGraphValues guiSidebar;

        private ContextMenu blankContextMenu;
        private ContextMenu keyframeContextMenu;
        private Vector2I contextClickPosition;

        private EdAnimationCurve[] curves = new EdAnimationCurve[0];

        private int markedFrameIdx;
        private List<KeyframeRef> selectedKeyframes = new List<KeyframeRef>();

        private bool isMousePressedOverKey;
        private KeyFrame[] draggedKeyframes;
        private Vector2 dragStart;

        public GUICurveEditor(GUILayout gui, int width, int height)
        {
            this.gui = gui;

            blankContextMenu = new ContextMenu();
            blankContextMenu.AddItem("Add keyframe", AddKeyframeAtPosition);
            blankContextMenu.AddItem("Add event", AddEventAtPosition);

            keyframeContextMenu = new ContextMenu();
            keyframeContextMenu.AddItem("Delete", DeleteSelectedKeyframes);
            keyframeContextMenu.AddItem("Tangents/Auto", () => { ChangeSelectionTangentMode(TangentMode.Auto); });
            keyframeContextMenu.AddItem("Tangents/Free", () => { ChangeSelectionTangentMode(TangentMode.Free); });
            keyframeContextMenu.AddItem("Tangents/In/Auto", () => { ChangeSelectionTangentMode(TangentMode.InAuto); });
            keyframeContextMenu.AddItem("Tangents/In/Free", () => { ChangeSelectionTangentMode(TangentMode.InFree); });
            keyframeContextMenu.AddItem("Tangents/In/Linear", () => { ChangeSelectionTangentMode(TangentMode.InLinear); });
            keyframeContextMenu.AddItem("Tangents/In/Step", () => { ChangeSelectionTangentMode(TangentMode.InStep); });
            keyframeContextMenu.AddItem("Tangents/Out/Auto", () => { ChangeSelectionTangentMode(TangentMode.OutAuto); });
            keyframeContextMenu.AddItem("Tangents/Out/Free", () => { ChangeSelectionTangentMode(TangentMode.OutFree); });
            keyframeContextMenu.AddItem("Tangents/Out/Linear", () => { ChangeSelectionTangentMode(TangentMode.OutLinear); });
            keyframeContextMenu.AddItem("Tangents/Out/Step", () => { ChangeSelectionTangentMode(TangentMode.OutStep); });

            guiTimeline = new GUIGraphTime(gui, width, TIMELINE_HEIGHT);

            drawingPanel = gui.AddPanel();
            drawingPanel.SetPosition(0, TIMELINE_HEIGHT);

            guiCurveDrawing = new GUICurveDrawing(drawingPanel, width, height - TIMELINE_HEIGHT, curves);
            guiCurveDrawing.SetRange(60.0f, 20.0f);

            GUIPanel sidebarPanel = gui.AddPanel(-10);
            sidebarPanel.SetPosition(0, TIMELINE_HEIGHT);

            guiSidebar = new GUIGraphValues(sidebarPanel, SIDEBAR_WIDTH, height - TIMELINE_HEIGHT);
            guiSidebar.SetRange(-10.0f, 10.0f);
        }

        /// <summary>
        /// Change the set of curves to display.
        /// </summary>
        /// <param name="curves">New set of curves to draw on the GUI element.</param>
        public void SetCurves(EdAnimationCurve[] curves)
        {
            this.curves = curves;
            guiCurveDrawing.SetCurves(curves);

            // TODO - Recalculate valid size

            Redraw();
        }

        /// <summary>
        /// Change the physical size of the GUI element.
        /// </summary>
        /// <param name="width">Width of the element in pixels.</param>
        /// <param name="height">Height of the element in pixels.</param>
        public void SetSize(int width, int height)
        {
            guiTimeline.SetSize(width, TIMELINE_HEIGHT);
            guiCurveDrawing.SetSize(width, height - TIMELINE_HEIGHT);
            guiSidebar.SetSize(SIDEBAR_WIDTH, height - TIMELINE_HEIGHT);

            Redraw();
        }

        /// <summary>
        /// Changes the visible range that the GUI element displays.
        /// </summary>
        /// <param name="xRange">Range of the horizontal area. Displayed area will range from [0, xRange].</param>
        /// <param name="yRange">Range of the vertical area. Displayed area will range from 
        ///                      [-yRange * 0.5, yRange * 0.5]</param>
        public void SetRange(float xRange, float yRange)
        {
            guiTimeline.SetRange(xRange);
            guiCurveDrawing.SetRange(xRange, yRange);
            guiSidebar.SetRange(yRange * -0.5f, yRange * 0.5f);

            Redraw();
        }

        /// <summary>
        /// Number of frames per second, used for frame selection and marking.
        /// </summary>
        /// <param name="fps">Number of prames per second.</param>
        public void SetFPS(int fps)
        {
            guiTimeline.SetFPS(fps);
            guiCurveDrawing.SetFPS(fps);

            Redraw();
        }

        /// <summary>
        /// Sets the frame at which to display the frame marker.
        /// </summary>
        /// <param name="frameIdx">Index of the frame to display the marker on, or -1 to clear the marker.</param>
        public void SetMarkedFrame(int frameIdx)
        {
            markedFrameIdx = frameIdx;

            guiTimeline.SetMarkedFrame(frameIdx);
            guiCurveDrawing.SetMarkedFrame(frameIdx);

            Redraw();
        }

        public void AddKeyFrameAtMarker()
        {
            ClearSelection();

            foreach (var curve in curves)
            {
                float t = guiCurveDrawing.GetTimeForFrame(markedFrameIdx);
                float value = curve.Native.Evaluate(t);

                curve.AddKeyframe(t, value);
            }

            // TODO - UNDOREDO

            guiCurveDrawing.Rebuild();
        }

        public void Redraw()
        {
            guiCurveDrawing.Rebuild();
            guiTimeline.Rebuild();
            guiSidebar.Rebuild();
        }

        public void HandleInput(Vector2I pointerPos)
        {
            Rect2I drawingBounds = drawingPanel.Bounds;
            Vector2I drawingPos = pointerPos - new Vector2I(drawingBounds.x, drawingBounds.y);

            if (Input.IsPointerButtonDown(PointerButton.Left))
            {
                Vector2 curveCoord;
                int curveIdx;
                int keyIdx;
                if (guiCurveDrawing.GetCoordInfo(drawingPos, out curveCoord, out curveIdx, out keyIdx))
                {
                    if (keyIdx == -1)
                        ClearSelection();
                    else
                    {
                        if (!Input.IsButtonHeld(ButtonCode.LeftShift) && !Input.IsButtonHeld(ButtonCode.RightShift))
                            ClearSelection();

                        SelectKeyframe(curveIdx, keyIdx);

                        isMousePressedOverKey = true;
                        dragStart = curveCoord;
                    }

                    guiCurveDrawing.Rebuild();
                }
            }

            if (Input.IsPointerButtonHeld(PointerButton.Left))
            {
                if (isMousePressedOverKey)
                {
                    // TODO - Check if pointer moves some minimal amount
                    // - If so, start drag. Record all current positions
                    // - Calculate offset in curve space and apply to all keyframes
                }
                else
                {
                    int frameIdx = guiTimeline.GetFrame(pointerPos);

                    if (frameIdx != -1)
                        SetMarkedFrame(frameIdx);
                }
            }

            if (Input.IsPointerButtonUp(PointerButton.Left))
            {
                isMousePressedOverKey = false;
            }

            if (Input.IsPointerButtonDown(PointerButton.Right))
            {
                Vector2 curveCoord;
                int curveIdx;
                int keyIdx;
                if (guiCurveDrawing.GetCoordInfo(drawingPos, out curveCoord, out curveIdx, out keyIdx))
                {
                    contextClickPosition = pointerPos;

                    if (keyIdx == -1)
                    {
                        ClearSelection();

                        blankContextMenu.Open(contextClickPosition, gui);
                    }
                    else
                    {
                        // If clicked outside of current selection, just select the one keyframe
                        if (!IsSelected(curveIdx, keyIdx))
                        {
                            ClearSelection();
                            SelectKeyframe(curveIdx, keyIdx);

                            guiCurveDrawing.Rebuild();
                        }

                        keyframeContextMenu.Open(contextClickPosition, gui);
                    }
                }
            }

            if (Input.IsButtonUp(ButtonCode.Delete))
                DeleteSelectedKeyframes();
        }

        private void ChangeSelectionTangentMode(TangentMode mode)
        {
            // TODO - When changing just left or right, make sure to persist opposite tangent mode (or switch to equivalent broken mode if it wasn't broken)
        }

        private void AddKeyframeAtPosition()
        {
            // TODO
        }

        private void AddEventAtPosition()
        {
            // TODO
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

            // TODO - UNDOREDO

            ClearSelection();

            guiCurveDrawing.Rebuild();
        }

        private void ClearSelection()
        {
            guiCurveDrawing.ClearSelectedKeyframes();
            selectedKeyframes.Clear();
            isMousePressedOverKey = false;
        }

        private void SelectKeyframe(int curveIdx, int keyIdx)
        {
            guiCurveDrawing.SelectKeyframe(curveIdx, keyIdx, true);

            if (!IsSelected(curveIdx, keyIdx))
                selectedKeyframes.Add(new GUICurveEditor.KeyframeRef(curveIdx, keyIdx));
        }

        private bool IsSelected(int curveIdx, int keyIdx)
        {
            int existingIdx = selectedKeyframes.FindIndex(x =>
            {
                return x.curveIdx == curveIdx && x.keyIdx == keyIdx;
            });

            return (existingIdx != -1);
        }
    }

    /** @} */
}

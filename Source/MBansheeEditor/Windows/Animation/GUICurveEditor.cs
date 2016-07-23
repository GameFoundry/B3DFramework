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
        class SelectedKeyframes
        {
            public int curveIdx;
            public List<int> keyIndices = new List<int>();
        }

        struct DraggedKeyframe
        {
            public DraggedKeyframe(int index, KeyFrame original)
            {
                this.index = index;
                this.original = original;
            }

            public int index;
            public KeyFrame original;
        }

        class DraggedKeyframes
        {
            public int curveIdx;
            public List<DraggedKeyframe> keys = new List<DraggedKeyframe>();
        }

        private const int TIMELINE_HEIGHT = 20;
        private const int SIDEBAR_WIDTH = 30;
        private const int DRAG_START_DISTANCE = 3;

        private EditorWindow window;
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
        private List<SelectedKeyframes> selectedKeyframes = new List<SelectedKeyframes>();

        private bool isPointerHeld;
        private bool isMousePressedOverKey;
        private bool isMousePressedOverTangent;
        private bool isDragInProgress;
        private List<DraggedKeyframes> draggedKeyframes = new List<DraggedKeyframes>();
        private TangentRef draggedTangent;
        private Vector2I dragStart;

        public GUICurveEditor(EditorWindow window, GUILayout gui, int width, int height)
        {
            this.window = window;
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

            EditorInput.OnPointerPressed += OnPointerPressed;
            EditorInput.OnPointerMoved += OnPointerMoved;
            EditorInput.OnPointerReleased += OnPointerReleased;
            EditorInput.OnButtonUp += OnButtonUp;
        }

        private void OnPointerPressed(PointerEvent ev)
        {
            if (ev.IsUsed)
                return;

            Vector2I windowPos = window.ScreenToWindowPos(ev.ScreenPos);

            Rect2I elementBounds = GUIUtility.CalculateBounds(gui, window.GUI);
            Vector2I pointerPos = windowPos - new Vector2I(elementBounds.x, elementBounds.y);

            Rect2I drawingBounds = drawingPanel.Bounds;
            Vector2I drawingPos = pointerPos - new Vector2I(drawingBounds.x, drawingBounds.y);

            if (ev.Button == PointerButton.Left)
            {
                Vector2 curveCoord;
                if (guiCurveDrawing.PixelToCurveSpace(drawingPos, out curveCoord))
                {
                    KeyframeRef keyframeRef;
                    if (!guiCurveDrawing.FindKeyFrame(drawingPos, out keyframeRef))
                    {
                        ClearSelection();

                        TangentRef tangentRef;
                        if (guiCurveDrawing.FindTangent(drawingPos, out tangentRef))
                        {
                            isMousePressedOverTangent = true;
                            dragStart = drawingPos;
                            draggedTangent = tangentRef;
                        }
                    }
                    else
                    {
                        if (!IsSelected(keyframeRef))
                        {
                            if (!Input.IsButtonHeld(ButtonCode.LeftShift) && !Input.IsButtonHeld(ButtonCode.RightShift))
                                ClearSelection();

                            SelectKeyframe(keyframeRef);
                        }

                        isMousePressedOverKey = true;
                        dragStart = drawingPos;
                    }

                    guiCurveDrawing.Rebuild();
                }
                else
                {
                    int frameIdx = guiTimeline.GetFrame(pointerPos);

                    if (frameIdx != -1)
                        SetMarkedFrame(frameIdx);
                }

                isPointerHeld = true;
            }
            else if (ev.Button == PointerButton.Right)
            {
                Vector2 curveCoord;
                if (guiCurveDrawing.PixelToCurveSpace(drawingPos, out curveCoord))
                {
                    contextClickPosition = drawingPos;

                    KeyframeRef keyframeRef;
                    if (!guiCurveDrawing.FindKeyFrame(drawingPos, out keyframeRef))
                    {
                        ClearSelection();

                        blankContextMenu.Open(pointerPos, gui);
                    }
                    else
                    {
                        // If clicked outside of current selection, just select the one keyframe
                        if (!IsSelected(keyframeRef))
                        {
                            ClearSelection();
                            SelectKeyframe(keyframeRef);

                            guiCurveDrawing.Rebuild();
                        }

                        keyframeContextMenu.Open(pointerPos, gui);
                    }
                }
            }
        }

        private void OnPointerMoved(PointerEvent ev)
        {
            if (ev.Button != PointerButton.Left)
                return;

            if (isPointerHeld)
            {
                Vector2I windowPos = window.ScreenToWindowPos(ev.ScreenPos);

                Rect2I elementBounds = GUIUtility.CalculateBounds(gui, window.GUI);
                Vector2I pointerPos = windowPos - new Vector2I(elementBounds.x, elementBounds.y);

                if (isMousePressedOverKey || isMousePressedOverTangent)
                {
                    Rect2I drawingBounds = drawingPanel.Bounds;
                    Vector2I drawingPos = pointerPos - new Vector2I(drawingBounds.x, drawingBounds.y);

                    if (!isDragInProgress)
                    {
                        int distance = Vector2I.Distance(drawingPos, dragStart);
                        if (distance >= DRAG_START_DISTANCE)
                        {
                            if (isMousePressedOverKey)
                            {
                                draggedKeyframes.Clear();
                                foreach (var selectedEntry in selectedKeyframes)
                                {
                                    EdAnimationCurve curve = curves[selectedEntry.curveIdx];
                                    KeyFrame[] keyFrames = curve.KeyFrames;

                                    DraggedKeyframes newEntry = new DraggedKeyframes();
                                    draggedKeyframes.Add(newEntry);

                                    foreach (var keyframeIdx in selectedEntry.keyIndices)
                                        newEntry.keys.Add(new DraggedKeyframe(keyframeIdx, keyFrames[keyframeIdx]));
                                }
                            }

                            // TODO - UNDOREDO record keyframe or tangent

                            isDragInProgress = true;
                        }
                    }

                    if (isDragInProgress)
                    {
                        if (isMousePressedOverKey)
                        {
                            Vector2 diff = Vector2.Zero;

                            Vector2 dragStartCurve;
                            if (guiCurveDrawing.PixelToCurveSpace(dragStart, out dragStartCurve))
                            {
                                Vector2 currentPosCurve;
                                if (guiCurveDrawing.PixelToCurveSpace(drawingPos, out currentPosCurve))
                                    diff = currentPosCurve - dragStartCurve;
                            }

                            foreach (var draggedEntry in draggedKeyframes)
                            {
                                EdAnimationCurve curve = curves[draggedEntry.curveIdx];

                                for (int i = 0; i < draggedEntry.keys.Count; i++)
                                {
                                    DraggedKeyframe draggedKey = draggedEntry.keys[i];

                                    float newTime = draggedKey.original.time + diff.x;
                                    float newValue = draggedKey.original.value + diff.y;

                                    int newIndex = curve.UpdateKeyframe(draggedKey.index, newTime, newValue);

                                    // It's possible key changed position due to time change, but since we're moving all
                                    // keys at once they cannot change position relative to one another, otherwise we would
                                    // need to update indices for other keys as well.
                                    draggedKey.index = newIndex;
                                    draggedEntry.keys[i] = draggedKey;
                                }

                                curve.Apply();
                            }

                            // Rebuild selected keys from dragged keys (after potential sorting)
                            ClearSelection();
                            foreach (var draggedEntry in draggedKeyframes)
                            {
                                foreach (var keyframe in draggedEntry.keys)
                                    SelectKeyframe(new KeyframeRef(draggedEntry.curveIdx, keyframe.index));
                            }

                            guiCurveDrawing.Rebuild();
                        }
                        else if (isMousePressedOverTangent)
                        {
                            // TODO - Update tangent

                            guiCurveDrawing.Rebuild();
                        }
                    }
                }
                else // Move frame marker
                {
                    int frameIdx = guiTimeline.GetFrame(pointerPos);

                    if (frameIdx != -1)
                        SetMarkedFrame(frameIdx);
                }
            }
        }

        private void OnPointerReleased(PointerEvent ev)
        {
            isPointerHeld = false;
            isDragInProgress = false;
            isMousePressedOverKey = false;
            isMousePressedOverTangent = false;
        }

        private void OnButtonUp(ButtonEvent ev)
        {
            if(ev.Button == ButtonCode.Delete)
                DeleteSelectedKeyframes();
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
                float value = curve.Evaluate(t);

                curve.AddKeyframe(t, value);
                curve.Apply();
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
        
        private void ChangeSelectionTangentMode(TangentMode mode)
        {
            foreach (var selectedEntry in selectedKeyframes)
            {
                EdAnimationCurve curve = curves[selectedEntry.curveIdx];

                foreach (var keyframeIdx in selectedEntry.keyIndices)
                {
                    if (mode == TangentMode.Auto || mode == TangentMode.Free)
                        curve.SetTangentMode(keyframeIdx, mode);
                    else
                    {
                        TangentMode newMode = curve.TangentModes[keyframeIdx];

                        if (mode.HasFlag((TangentMode) TangentType.In))
                        {
                            // Replace only the in tangent mode, keeping the out tangent as is
                            TangentMode inFlags = (TangentMode.InAuto | TangentMode.InFree | TangentMode.InLinear |
                                                   TangentMode.InAuto);

                            newMode &= ~inFlags;
                            newMode |= (mode & inFlags);
                        }
                        else
                        {
                            // Replace only the out tangent mode, keeping the in tangent as is
                            TangentMode outFlags = (TangentMode.OutAuto | TangentMode.OutFree | TangentMode.OutLinear |
                                                    TangentMode.OutAuto);

                            newMode &= ~outFlags;
                            newMode |= (mode & outFlags);
                        }

                        curve.SetTangentMode(keyframeIdx, newMode);
                    }
                }

                curve.Apply();
            }

            // TODO - UNDOREDO

            guiCurveDrawing.Rebuild();
        }

        private void AddKeyframeAtPosition()
        {
            Vector2 curveCoord;
            if (guiCurveDrawing.PixelToCurveSpace(contextClickPosition, out curveCoord))
            {
                ClearSelection();

                foreach (var curve in curves)
                {
                    float t = curveCoord.x;
                    float value = curveCoord.y;

                    curve.AddKeyframe(t, value);
                    curve.Apply();
                }

                // TODO - UNDOREDO

                guiCurveDrawing.Rebuild();
            }
        }

        private void AddEventAtPosition()
        {
            // TODO
        }

        private void DeleteSelectedKeyframes()
        {
            foreach (var selectedEntry in selectedKeyframes)
            {
                EdAnimationCurve curve = curves[selectedEntry.curveIdx];

                // Sort keys from highest to lowest so the indices don't change
                selectedEntry.keyIndices.Sort((x, y) =>
                {
                    return y.CompareTo(x);
                });

                foreach (var keyframeIdx in selectedEntry.keyIndices)
                    curve.RemoveKeyframe(keyframeIdx);

                curve.Apply();
            }

            // TODO - UNDOREDO

            ClearSelection();

            guiCurveDrawing.Rebuild();
        }

        private void ClearSelection()
        {
            guiCurveDrawing.ClearSelectedKeyframes();
            selectedKeyframes.Clear();
        }

        private void SelectKeyframe(KeyframeRef keyframeRef)
        {
            guiCurveDrawing.SelectKeyframe(keyframeRef, true);

            if (!IsSelected(keyframeRef))
            {
                int curveIdx = selectedKeyframes.FindIndex(x =>
                {
                    return x.curveIdx == keyframeRef.curveIdx;
                });

                if (curveIdx == -1)
                {
                    curveIdx = selectedKeyframes.Count;
                    selectedKeyframes.Add(new SelectedKeyframes());
                }

                selectedKeyframes[curveIdx].keyIndices.Add(keyframeRef.keyIdx);
            }
        }

        private bool IsSelected(KeyframeRef keyframeRef)
        {
            int curveIdx = selectedKeyframes.FindIndex(x =>
            {
                return x.curveIdx == keyframeRef.curveIdx;
            });

            if (curveIdx == -1)
                return false;

            int keyIdx = selectedKeyframes[curveIdx].keyIndices.FindIndex(x =>
            {
                return x == keyframeRef.keyIdx;
            });

            return keyIdx != -1;
        }
    }

    /** @} */
}

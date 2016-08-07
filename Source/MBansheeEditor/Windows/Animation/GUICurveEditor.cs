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
        private const int EVENTS_HEIGHT = 10;
        private const int SIDEBAR_WIDTH = 30;
        private const int DRAG_START_DISTANCE = 3;

        private EditorWindow window;
        private GUILayout gui;
        private GUIPanel drawingPanel;

        private GUIGraphTime guiTimeline;
        private GUIAnimEvents guiEvents;
        private GUICurveDrawing guiCurveDrawing;
        private GUIGraphValues guiSidebar;

        private ContextMenu blankContextMenu;
        private ContextMenu keyframeContextMenu;
        private Vector2I contextClickPosition;

        private EdAnimationCurve[] curves = new EdAnimationCurve[0];
        private float xRange = 60.0f;
        private float yRange = 10.0f;
        private Vector2 offset;

        private int width;
        private int height;

        private int markedFrameIdx;
        private List<SelectedKeyframes> selectedKeyframes = new List<SelectedKeyframes>();

        private bool isPointerHeld;
        private bool isMousePressedOverKey;
        private bool isMousePressedOverTangent;
        private bool isDragInProgress;
        private List<DraggedKeyframes> draggedKeyframes = new List<DraggedKeyframes>();
        private TangentRef draggedTangent;
        private Vector2I dragStart;

        /// <summary>
        /// Triggers whenever user selects a new frame.
        /// </summary>
        public Action<int> OnFrameSelected;

        /// <summary>
        /// The displayed range of the curve, where:
        ///   .x - Range of the horizontal area. Displayed area ranges from [0, x].
        ///   .y - Range of the vertical area. Displayed area ranges from [-y, y].                 
        /// </summary>
        public Vector2 Range
        {
            get { return new Vector2(xRange, yRange); }
            set
            {
                xRange = value.x;
                yRange = value.y;

                guiTimeline.SetRange(xRange);
                guiEvents.SetRange(xRange);
                guiCurveDrawing.SetRange(xRange, yRange * 2.0f);
                guiSidebar.SetRange(offset.y - yRange, offset.y + yRange);

                Redraw();
            }
        }

        /// <summary>
        /// Returns the offset of the displayed curve values.
        /// </summary>
        public Vector2 Offset
        {
            get { return offset; }
            set
            {
                offset = value;

                guiTimeline.SetOffset(offset.x);
                guiEvents.SetOffset(offset.x);
                guiCurveDrawing.SetOffset(offset);
                guiSidebar.SetRange(offset.y - yRange, offset.y + yRange);

                Redraw();
            }
        }

        /// <summary>
        /// Returns the width of the curve editor, in pixels.
        /// </summary>
        public int Width
        {
            get { return width; }
        }

        /// <summary>
        /// Returns the height of the curve editor, in pixels.
        /// </summary>
        public int Height
        {
            get { return height; }
        }

        public GUICurveEditor(EditorWindow window, GUILayout gui, int width, int height)
        {
            this.window = window;
            this.gui = gui;

            this.width = width;
            this.height = height;

            blankContextMenu = new ContextMenu();
            blankContextMenu.AddItem("Add keyframe", AddKeyframeAtPosition);

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

            GUIPanel eventsPanel = gui.AddPanel();
            eventsPanel.SetPosition(0, TIMELINE_HEIGHT);
            guiEvents = new GUIAnimEvents(eventsPanel, width, EVENTS_HEIGHT);
            
            drawingPanel = gui.AddPanel();
            drawingPanel.SetPosition(0, TIMELINE_HEIGHT + EVENTS_HEIGHT);

            guiCurveDrawing = new GUICurveDrawing(drawingPanel, width, height - TIMELINE_HEIGHT - EVENTS_HEIGHT, curves);
            guiCurveDrawing.SetRange(60.0f, 20.0f);

            GUIPanel sidebarPanel = gui.AddPanel(-10);
            sidebarPanel.SetPosition(0, TIMELINE_HEIGHT + EVENTS_HEIGHT);

            guiSidebar = new GUIGraphValues(sidebarPanel, SIDEBAR_WIDTH, height - TIMELINE_HEIGHT - EVENTS_HEIGHT);
            guiSidebar.SetRange(-10.0f, 10.0f);
        }

        /// <summary>
        /// Converts pixel coordinates relative to the curve drawing area into coordinates in curve space.
        /// </summary>
        /// <param name="pixelCoords">Coordinates relative to this GUI element, in pixels.</param>
        /// <param name="curveCoords">Curve coordinates within the range as specified by <see cref="Range"/>. Only
        ///                           valid when function returns true.</param>
        /// <returns>True if the coordinates are within the curve area, false otherwise.</returns>
        public bool PixelToCurveSpace(Vector2I pixelCoords, out Vector2 curveCoords)
        {
            return guiCurveDrawing.PixelToCurveSpace(pixelCoords, out curveCoords);
        }

        /// <summary>
        /// Converts coordinate in curve space (time, value) into pixel coordinates relative to the curve drawing area
        /// origin.
        /// </summary>
        /// <param name="curveCoords">Time and value of the location to convert.</param>
        /// <returns>Coordinates relative to curve drawing area's origin, in pixels.</returns>
        public Vector2I CurveToPixelSpace(Vector2 curveCoords)
        {
            return guiCurveDrawing.CurveToPixelSpace(curveCoords);
        }

        public bool WindowToCurveSpace(Vector2I windowPos, out Vector2 curveCoord)
        {
            Rect2I elementBounds = GUIUtility.CalculateBounds(gui, window.GUI);
            Vector2I pointerPos = windowPos - new Vector2I(elementBounds.x, elementBounds.y);

            Rect2I drawingBounds = drawingPanel.Bounds;
            Vector2I drawingPos = pointerPos - new Vector2I(drawingBounds.x, drawingBounds.y);

            return guiCurveDrawing.PixelToCurveSpace(drawingPos, out curveCoord);
        }

        internal void OnPointerPressed(PointerEvent ev)
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
                        TangentRef tangentRef;
                        if (guiCurveDrawing.FindTangent(drawingPos, out tangentRef))
                        {
                            isMousePressedOverTangent = true;
                            dragStart = drawingPos;
                            draggedTangent = tangentRef;
                        }
                        else
                            ClearSelection();
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
                    else
                    {
                        int eventIdx;
                        if (guiEvents.FindEvent(pointerPos, out eventIdx))
                        {
                            // TODO - Select event
                        }
                    }

                    OnFrameSelected?.Invoke(frameIdx);
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

        internal void OnPointerMoved(PointerEvent ev)
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
                                    newEntry.curveIdx = selectedEntry.curveIdx;
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
                            EdAnimationCurve curve = curves[draggedTangent.keyframeRef.curveIdx];
                            KeyFrame keyframe = curve.KeyFrames[draggedTangent.keyframeRef.keyIdx];

                            Vector2 keyframeCurveCoords = new Vector2(keyframe.time, keyframe.value);

                            Vector2 currentPosCurve;
                            if (guiCurveDrawing.PixelToCurveSpace(drawingPos, out currentPosCurve))
                            {
                                Vector2 normal = currentPosCurve - keyframeCurveCoords;
                                normal = normal.Normalized;

                                float tangent = EdAnimationCurve.NormalToTangent(normal);

                                if (draggedTangent.type == TangentType.In)
                                {
                                    if (normal.x > 0.0f)
                                        tangent = float.PositiveInfinity;

                                    keyframe.inTangent = tangent;
                                }
                                else
                                {
                                    if (normal.x < 0.0f)
                                        tangent = float.PositiveInfinity;

                                    keyframe.outTangent = tangent;
                                }

                                curve.KeyFrames[draggedTangent.keyframeRef.keyIdx] = keyframe;
                                curve.Apply();

                                guiCurveDrawing.Rebuild();
                            }
                        }
                    }
                }
                else // Move frame marker
                {
                    int frameIdx = guiTimeline.GetFrame(pointerPos);

                    if (frameIdx != -1)
                        SetMarkedFrame(frameIdx);

                    OnFrameSelected?.Invoke(frameIdx);
                }
            }
        }

        internal void OnPointerReleased(PointerEvent ev)
        {
            isPointerHeld = false;
            isDragInProgress = false;
            isMousePressedOverKey = false;
            isMousePressedOverTangent = false;
        }

        internal void OnButtonUp(ButtonEvent ev)
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

            Redraw();
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

            guiTimeline.SetSize(width, TIMELINE_HEIGHT);
            guiEvents.SetSize(height, EVENTS_HEIGHT);
            guiCurveDrawing.SetSize(width, height - TIMELINE_HEIGHT - EVENTS_HEIGHT);
            guiSidebar.SetSize(SIDEBAR_WIDTH, height - TIMELINE_HEIGHT - EVENTS_HEIGHT);

            Redraw();
        }

        /// <summary>
        /// Number of frames per second, used for frame selection and marking.
        /// </summary>
        /// <param name="fps">Number of prames per second.</param>
        public void SetFPS(int fps)
        {
            guiTimeline.SetFPS(fps);
            guiEvents.SetFPS(fps);
            guiCurveDrawing.SetFPS(fps);

            Redraw();
        }

        /// <summary>
        /// Returns time for a frame with the specified index. Depends on set range and FPS.
        /// </summary>
        /// <param name="frameIdx">Index of the frame (not a key-frame) to get the time for.</param>
        /// <returns>Time of the frame with the provided index. </returns>
        public float GetTimeForFrame(int frameIdx)
        {
            return guiCurveDrawing.GetTimeForFrame(markedFrameIdx);
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
            guiEvents.Rebuild();
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

                    SelectedKeyframes newKeyframes = new SelectedKeyframes();
                    newKeyframes.curveIdx = keyframeRef.curveIdx;

                    selectedKeyframes.Add(newKeyframes);
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

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
    /// Displays a set of animation curves and events. Allows manipulation of both by adding, removing and modifying
    /// curve keyframes, and animation events.
    /// </summary>
    internal class GUICurveEditor
    {
        /// <summary>
        /// Information about currently selected set of keyframes for a specific curve.
        /// </summary>
        class SelectedKeyframes
        {
            public int curveIdx;
            public List<int> keyIndices = new List<int>();
        }

        /// <summary>
        /// Information about a keyframe that is currently being dragged.
        /// </summary>
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

        /// <summary>
        /// Information about all keyframes of a specific curve that are currently being dragged.
        /// </summary>
        class DraggedKeyframes
        {
            public int curveIdx;
            public List<DraggedKeyframe> keys = new List<DraggedKeyframe>();
        }

        /// <summary>
        /// Data about an animation event.
        /// </summary>
        class EventInfo
        {
            public AnimationEvent animEvent;
            public bool selected;
        }

        private const int TIMELINE_HEIGHT = 20;
        private const int EVENTS_HEIGHT = 10;
        private const int SIDEBAR_WIDTH = 30;
        private const int DRAG_START_DISTANCE = 3;

        private AnimationWindow window;
        private GUILayout gui;
        private GUIPanel drawingPanel;
        private GUIPanel eventsPanel;

        private GUIGraphTime guiTimeline;
        private GUIAnimEvents guiEvents;
        private GUICurveDrawing guiCurveDrawing;
        private GUIGraphValues guiSidebar;

        private ContextMenu blankContextMenu;
        private ContextMenu keyframeContextMenu;
        private ContextMenu blankEventContextMenu;
        private ContextMenu eventContextMenu;
        private Vector2I contextClickPosition;

        private EdAnimationCurve[] curves = new EdAnimationCurve[0];
        private bool disableCurveEdit = false;

        private float xRange = 60.0f;
        private float yRange = 10.0f;
        private Vector2 offset;

        private int width;
        private int height;

        private int markedFrameIdx;
        private List<SelectedKeyframes> selectedKeyframes = new List<SelectedKeyframes>();

        private List<EventInfo> events = new List<EventInfo>();

        private bool isPointerHeld;
        private bool isMousePressedOverKey;
        private bool isMousePressedOverTangent;
        private bool isDragInProgress;
        private List<DraggedKeyframes> draggedKeyframes = new List<DraggedKeyframes>();
        private TangentRef draggedTangent;
        private Vector2I dragStart;

        /// <summary>
        /// Triggers whenever user selects a new frame. Reports the index of the selected frame.
        /// </summary>
        public Action<int> OnFrameSelected;

        /// <summary>
        /// Triggered whenever a new animation event is added.
        /// </summary>
        public Action OnEventAdded;

        /// <summary>
        /// Triggered whenever values in an animation event change.
        /// </summary>
        public Action OnEventModified;

        /// <summary>
        /// Triggered whenever an animation event is deleted.
        /// </summary>
        public Action OnEventDeleted;

        /// <summary>
        /// Triggered whenever keyframe in a curve is modified (added, removed or edited).
        /// </summary>
        public Action OnCurveModified;

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
        /// Determines how much to offset the displayed curve values.
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

        /// <summary>
        /// Set to true if curves are not allowed to be edited.
        /// </summary>
        public bool DisableCurveEdit
        {
            set { disableCurveEdit = value; }
        }

        /// <summary>
        /// Animation events displayed on the curve editor.
        /// </summary>
        public AnimationEvent[] Events
        {
            get
            {
                AnimationEvent[] animEvents = new AnimationEvent[events.Count];

                // Note: Hidden dependency. Returned events must point to the same event class this object is using, so
                // that any modifications made in this class will be visible in the returned values.
                for (int i = 0; i < events.Count; i++)
                    animEvents[i] = events[i].animEvent;

                return animEvents;
            }

            set
            {
                events.Clear();

                for (int i = 0; i < value.Length; i++)
                {
                    EventInfo eventInfo = new EventInfo();
                    eventInfo.animEvent = value[i];

                    events.Add(eventInfo);
                }

                UpdateEventsGUI();
            }
        }

        /// <summary>
        /// Creates a new curve editor GUI elements.
        /// </summary>
        /// <param name="window">Parent window of the GUI element.</param>
        /// <param name="gui">GUI layout into which to place the GUI element.</param>
        /// <param name="width">Width in pixels.</param>
        /// <param name="height">Height in pixels.</param>
        public GUICurveEditor(AnimationWindow window, GUILayout gui, int width, int height)
        {
            this.window = window;
            this.gui = gui;

            this.width = width;
            this.height = height;

            blankContextMenu = new ContextMenu();
            blankContextMenu.AddItem("Add keyframe", AddKeyframeAtPosition);

            blankEventContextMenu = new ContextMenu();
            blankEventContextMenu.AddItem("Add event", AddEventAtPosition);

            keyframeContextMenu = new ContextMenu();
            keyframeContextMenu.AddItem("Delete", DeleteSelectedKeyframes);
            keyframeContextMenu.AddItem("Delete", EditSelectedKeyframe);
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

            eventContextMenu = new ContextMenu();
            eventContextMenu.AddItem("Delete", DeleteSelectedEvents);
            eventContextMenu.AddItem("Edit", EditSelectedEvent);

            guiTimeline = new GUIGraphTime(gui, width, TIMELINE_HEIGHT);

            eventsPanel = gui.AddPanel();
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

        /// <summary>
        /// Converts coordinates in window space (relative to the parent window origin) into coordinates in curve space.
        /// </summary>
        /// <param name="windowPos">Coordinates relative to parent editor window, in pixels.</param>
        /// <param name="curveCoord">Curve coordinates within the range as specified by <see cref="Range"/>. Only
        ///                          valid when function returns true.</param>
        /// <returns>True if the coordinates are within the curve area, false otherwise.</returns>
        public bool WindowToCurveSpace(Vector2I windowPos, out Vector2 curveCoord)
        {
            Rect2I elementBounds = GUIUtility.CalculateBounds(gui, window.GUI);
            Vector2I pointerPos = windowPos - new Vector2I(elementBounds.x, elementBounds.y);

            Rect2I drawingBounds = drawingPanel.Bounds;
            Vector2I drawingPos = pointerPos - new Vector2I(drawingBounds.x, drawingBounds.y);

            return guiCurveDrawing.PixelToCurveSpace(drawingPos, out curveCoord);
        }

        /// <summary>
        /// Handles input. Should be called by the owning window whenever a pointer is pressed.
        /// </summary>
        /// <param name="ev">Object containing pointer press event information.</param>
        internal void OnPointerPressed(PointerEvent ev)
        {
            if (ev.IsUsed)
                return;

            Vector2I windowPos = window.ScreenToWindowPos(ev.ScreenPos);

            Rect2I elementBounds = GUIUtility.CalculateBounds(gui, window.GUI);
            Vector2I pointerPos = windowPos - new Vector2I(elementBounds.x, elementBounds.y);

            Rect2I drawingBounds = drawingPanel.Bounds;
            Vector2I drawingPos = pointerPos - new Vector2I(drawingBounds.x, drawingBounds.y);

            Rect2I eventBounds = eventsPanel.Bounds;
            Vector2I eventPos = pointerPos - new Vector2I(eventBounds.x, eventBounds.y);

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
                    UpdateEventsGUI();
                }
                else
                {
                    int frameIdx = guiTimeline.GetFrame(pointerPos);

                    if (frameIdx != -1)
                        SetMarkedFrame(frameIdx);
                    else
                    {
                        int eventIdx;
                        if (guiEvents.FindEvent(eventPos, out eventIdx))
                        {
                            if (!Input.IsButtonHeld(ButtonCode.LeftShift) && !Input.IsButtonHeld(ButtonCode.RightShift))
                                ClearSelection();

                            events[eventIdx].selected = true;
                            UpdateEventsGUI();
                        }
                        else
                        {
                            ClearSelection();

                            guiCurveDrawing.Rebuild();
                            UpdateEventsGUI();
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

                        guiCurveDrawing.Rebuild();
                        UpdateEventsGUI();
                    }
                    else
                    {
                        // If clicked outside of current selection, just select the one keyframe
                        if (!IsSelected(keyframeRef))
                        {
                            ClearSelection();
                            SelectKeyframe(keyframeRef);

                            guiCurveDrawing.Rebuild();
                            UpdateEventsGUI();
                        }

                        keyframeContextMenu.Open(pointerPos, gui);
                    }
                }
                else if (guiEvents.GetFrame(eventPos) != -1) // Clicked over events bar
                {
                    contextClickPosition = eventPos;

                    int eventIdx;
                    if (!guiEvents.FindEvent(eventPos, out eventIdx))
                    {
                        ClearSelection();

                        blankEventContextMenu.Open(pointerPos, gui);

                        guiCurveDrawing.Rebuild();
                        UpdateEventsGUI();
                    }
                    else
                    {
                        // If clicked outside of current selection, just select the one event
                        if (!events[eventIdx].selected)
                        {
                            ClearSelection();
                            events[eventIdx].selected = true;

                            guiCurveDrawing.Rebuild();
                            UpdateEventsGUI();
                        }

                        eventContextMenu.Open(pointerPos, gui);
                    }
                }
            }
        }

        /// <summary>
        /// Handles input. Should be called by the owning window whenever a pointer is moved.
        /// </summary>
        /// <param name="ev">Object containing pointer move event information.</param>
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
                            if (isMousePressedOverKey && !disableCurveEdit)
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
                        if (isMousePressedOverKey && !disableCurveEdit)
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

                            OnCurveModified?.Invoke();
                            guiCurveDrawing.Rebuild();
                            UpdateEventsGUI();
                        }
                        else if (isMousePressedOverTangent && !disableCurveEdit)
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

                                OnCurveModified?.Invoke();
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

        /// <summary>
        /// Handles input. Should be called by the owning window whenever a pointer is released.
        /// </summary>
        /// <param name="ev">Object containing pointer release event information.</param>
        internal void OnPointerReleased(PointerEvent ev)
        {
            isPointerHeld = false;
            isDragInProgress = false;
            isMousePressedOverKey = false;
            isMousePressedOverTangent = false;
        }

        /// <summary>
        /// Handles input. Should be called by the owning window whenever a button is released.
        /// </summary>
        /// <param name="ev">Object containing button release event information.</param>
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
            guiEvents.SetMarkedFrame(frameIdx);
            guiCurveDrawing.SetMarkedFrame(frameIdx);

            Redraw();
        }

        /// <summary>
        /// Adds a new keyframe at the currently selected frame.
        /// </summary>
        public void AddKeyFrameAtMarker()
        {
            ClearSelection();

            if (!disableCurveEdit)
            {
                foreach (var curve in curves)
                {
                    float t = guiCurveDrawing.GetTimeForFrame(markedFrameIdx);
                    float value = curve.Evaluate(t);

                    curve.AddKeyframe(t, value);
                    curve.Apply();
                }
            }

            // TODO - UNDOREDO

            OnCurveModified?.Invoke();
            guiCurveDrawing.Rebuild();
            UpdateEventsGUI();
        }

        /// <summary>
        /// Adds a new event at the currently selected event.
        /// </summary>
        public void AddEventAtMarker()
        {
            ClearSelection();

            float eventTime = guiEvents.GetTimeForFrame(markedFrameIdx);
            EventInfo eventInfo = new EventInfo();
            eventInfo.animEvent = new AnimationEvent("", eventTime);
            
            events.Add(eventInfo); // TODO - UNDOREDO
            OnEventAdded?.Invoke();

            UpdateEventsGUI();
            guiCurveDrawing.Rebuild();

            StartEventEdit(events.Count - 1);
        }

        /// <summary>
        /// Rebuilds GUI displaying the animation events list.
        /// </summary>
        private void UpdateEventsGUI()
        {
            AnimationEvent[] animEvents = new AnimationEvent[events.Count];
            bool[] selected = new bool[events.Count];

            for (int i = 0; i < events.Count; i++)
            {
                animEvents[i] = events[i].animEvent;
                selected[i] = events[i].selected;
            }

            guiEvents.SetEvents(animEvents, selected);
            guiEvents.Rebuild();
        }

        /// <summary>
        /// Rebuilds the entire curve editor GUI.
        /// </summary>
        public void Redraw()
        {
            guiCurveDrawing.Rebuild();
            guiTimeline.Rebuild();
            guiEvents.Rebuild();
            guiSidebar.Rebuild();
        }
        
        /// <summary>
        /// Changes the tangent mode for all currently selected keyframes.
        /// </summary>
        /// <param name="mode">Tangent mode to set. If only in or out tangent mode is provided, the mode for the opposite 
        ///                    tangent will be kept as is.</param>
        private void ChangeSelectionTangentMode(TangentMode mode)
        {
            if (disableCurveEdit)
                return;

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

            OnCurveModified?.Invoke();
            guiCurveDrawing.Rebuild();
        }

        /// <summary>
        /// Adds a new keyframe at the position the context menu was opened at.
        /// </summary>
        private void AddKeyframeAtPosition()
        {
            Vector2 curveCoord;
            if (guiCurveDrawing.PixelToCurveSpace(contextClickPosition, out curveCoord))
            {
                ClearSelection();

                if (!disableCurveEdit)
                {
                    foreach (var curve in curves)
                    {
                        float t = curveCoord.x;
                        float value = curveCoord.y;

                        curve.AddKeyframe(t, value);
                        curve.Apply();
                    }
                }

                // TODO - UNDOREDO

                OnCurveModified?.Invoke();
                guiCurveDrawing.Rebuild();
                UpdateEventsGUI();
            }
        }

        /// <summary>
        /// Adds a new event at the position the context menu was opened at.
        /// </summary>
        private void AddEventAtPosition()
        {
            int frame = guiEvents.GetFrame(contextClickPosition);
            if (frame != -1)
            {
                ClearSelection();

                float time = guiEvents.GetTimeForFrame(frame);

                EventInfo eventInfo = new EventInfo();
                eventInfo.animEvent = new AnimationEvent("", time);

                events.Add(eventInfo); // TODO - UNDOREDO
                OnEventAdded?.Invoke();

                UpdateEventsGUI();
                guiCurveDrawing.Rebuild();

                StartEventEdit(events.Count - 1);
            }
        }

        /// <summary>
        /// Removes all currently selected keyframes from the curves.
        /// </summary>
        private void DeleteSelectedKeyframes()
        {
            if (!disableCurveEdit)
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
            }

            // TODO - UNDOREDO

            ClearSelection();

            OnCurveModified?.Invoke();
            guiCurveDrawing.Rebuild();
            UpdateEventsGUI();
        }

        /// <summary>
        /// Deletes all currently selected events.
        /// </summary>
        private void DeleteSelectedEvents()
        {
            List<EventInfo> newEvents = new List<EventInfo>();
            foreach (var entry in events)
            {
                if(!entry.selected)
                    newEvents.Add(entry);
            }

            events = newEvents; // TODO - UNDOREDO
            OnEventDeleted?.Invoke();

            ClearSelection();

            guiCurveDrawing.Rebuild();
            UpdateEventsGUI();
        }

        /// <summary>
        /// Unselects any selected keyframes and events.
        /// </summary>
        private void ClearSelection()
        {
            guiCurveDrawing.ClearSelectedKeyframes();
            selectedKeyframes.Clear();

            foreach (var entry in events)
                entry.selected = false;
        }

        /// <summary>
        /// Adds the provided keyframe to the selection list (doesn't clear existing ones).
        /// </summary>
        /// <param name="keyframeRef">Keyframe to select.</param>
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

        /// <summary>
        /// Checks is the provided keyframe currently selected.
        /// </summary>
        /// <param name="keyframeRef">Keyframe to check.</param>
        /// <returns>True if selected, false otherwise.</returns>
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

        /// <summary>
        /// Opens the edit window for the currently selected keyframe.
        /// </summary>
        private void EditSelectedKeyframe()
        {
            if (disableCurveEdit)
                return;

            if (selectedKeyframes.Count == 0)
                return;

            EdAnimationCurve curve = curves[selectedKeyframes[0].curveIdx];
            KeyFrame[] keyFrames = curve.KeyFrames;

            int keyIndex = selectedKeyframes[0].keyIndices[0];
            KeyFrame keyFrame = keyFrames[keyIndex];
            Vector2I position = guiCurveDrawing.CurveToPixelSpace(new Vector2(keyFrame.time, keyFrame.value));

            Rect2I drawingBounds = GUIUtility.CalculateBounds(drawingPanel, window.GUI);
            position.x = MathEx.Clamp(position.x, 0, drawingBounds.width);
            position.y = MathEx.Clamp(position.y, 0, drawingBounds.height);

            Vector2I windowPos = position + new Vector2I(drawingBounds.x, drawingBounds.y);
            
            KeyframeEditWindow editWindow = DropDownWindow.Open<KeyframeEditWindow>(window, windowPos);
            editWindow.Initialize(keyFrame, x =>
            {
                curve.UpdateKeyframe(keyIndex, x.time, x.value);
                curve.Apply();
                // TODO UNDOREDO

                guiCurveDrawing.Rebuild();
                OnCurveModified?.Invoke();
            });
        }

        /// <summary>
        /// Opens the edit window for the currently selected event.
        /// </summary>
        private void EditSelectedEvent()
        {
            for (int i = 0; i < events.Count; i++)
            {
                if (events[i].selected)
                {
                    StartEventEdit(i);
                    break;
                }
            }
        }

        /// <summary>
        /// Opens the event edit window for the specified event.
        /// </summary>
        /// <param name="eventIdx">Event index to open the edit window for.</param>
        private void StartEventEdit(int eventIdx)
        {
            AnimationEvent animEvent = events[eventIdx].animEvent;

            Vector2I position = new Vector2I();
            position.x = guiEvents.GetOffset(animEvent.Time);
            position.y = EVENTS_HEIGHT/2;

            Rect2I eventBounds = GUIUtility.CalculateBounds(eventsPanel, window.GUI);
            Vector2I windowPos = position + new Vector2I(eventBounds.x, eventBounds.y);

            SceneObject so = window.SelectedSO;
            Component[] components = so.GetComponents();
            string[] componentNames = new string[components.Length];
            for (int i = 0; i < components.Length; i++)
                componentNames[i] = components[i].GetType().Name;

            EventEditWindow editWindow = DropDownWindow.Open<EventEditWindow>(window, windowPos);
            editWindow.Initialize(animEvent, componentNames, () =>
            {
                UpdateEventsGUI();
                OnEventModified?.Invoke();
            });
        }
    }

    /// <summary>
    /// Drop down window that displays input boxes used for editing a keyframe.
    /// </summary>
    [DefaultSize(120, 80)]
    internal class KeyframeEditWindow : DropDownWindow
    {
        /// <summary>
        /// Initializes the drop down window by creating the necessary GUI. Must be called after construction and before
        /// use.
        /// </summary>
        /// <param name="keyFrame">Keyframe whose properties to edit.</param>
        /// <param name="updateCallback">Callback triggered when event values change.</param>
        internal void Initialize(KeyFrame keyFrame, Action<KeyFrame> updateCallback)
        {
            GUIFloatField timeField = new GUIFloatField(new LocEdString("Time"), 40, "");
            timeField.Value = keyFrame.time;
            timeField.OnChanged += x => { keyFrame.time = x; updateCallback(keyFrame); };

            GUIFloatField valueField = new GUIFloatField(new LocEdString("Value"), 40, "");
            valueField.Value = keyFrame.value;
            valueField.OnChanged += x => { keyFrame.value = x; updateCallback(keyFrame); };

            GUILayoutY vertLayout = GUI.AddLayoutY();

            vertLayout.AddFlexibleSpace();
            GUILayoutX horzLayout = vertLayout.AddLayoutX();
            horzLayout.AddFlexibleSpace();
            GUILayout contentLayout = horzLayout.AddLayoutY();
            GUILayout timeLayout = contentLayout.AddLayoutX();
            timeLayout.AddSpace(5);
            timeLayout.AddElement(timeField);
            timeLayout.AddFlexibleSpace();
            GUILayout componentLayout = contentLayout.AddLayoutX();
            componentLayout.AddSpace(5);
            componentLayout.AddElement(valueField);
            componentLayout.AddFlexibleSpace();
            horzLayout.AddFlexibleSpace();
            vertLayout.AddFlexibleSpace();
        }
    }

    /// <summary>
    /// Drop down window that displays input boxes used for editing an event.
    /// </summary>
    [DefaultSize(200, 80)]
    internal class EventEditWindow : DropDownWindow
    {
        /// <summary>
        /// Initializes the drop down window by creating the necessary GUI. Must be called after construction and before
        /// use.
        /// </summary>
        /// <param name="animEvent">Event whose properties to edit.</param>
        /// <param name="componentNames">List of component names that the user can select from.</param>
        /// <param name="updateCallback">Callback triggered when event values change.</param>
        internal void Initialize(AnimationEvent animEvent, string[] componentNames, Action updateCallback)
        {
            int selectedIndex = -1;
            if (!string.IsNullOrEmpty(animEvent.Name))
            {
                string[] nameEntries = animEvent.Name.Split('/');
                if (nameEntries.Length > 1)
                {
                    string typeName = nameEntries[0];
                    for (int i = 0; i < componentNames.Length; i++)
                    {
                        if (componentNames[i] == typeName)
                        {
                            selectedIndex = i;
                            break;
                        }
                    }
                }
            }

            GUIFloatField timeField = new GUIFloatField(new LocEdString("Time"), 40, "");
            timeField.Value = animEvent.Time;
            timeField.OnChanged += x => { animEvent.Time = x; updateCallback(); }; // TODO UNDOREDO  

            GUIListBoxField componentField = new GUIListBoxField(componentNames, new LocEdString("Component"), 40);
            if (selectedIndex != -1)
                componentField.Index = selectedIndex;

            componentField.OnSelectionChanged += x =>
            {
                string compName = "";
                if (x != -1)
                    compName = componentNames[x] + "/";

                animEvent.Name = compName + x;
                updateCallback();
            };// TODO UNDOREDO 

            GUITextField methodField = new GUITextField(new LocEdString("Method"), 40, false, "", GUIOption.FixedWidth(190));
            methodField.Value = animEvent.Name;
            methodField.OnChanged += x =>
            {
                string compName = "";
                if(componentField.Index != -1)
                    compName = componentNames[componentField.Index] + "/";

                animEvent.Name = compName + x;
                updateCallback();
            }; // TODO UNDOREDO 

            GUILayoutY vertLayout = GUI.AddLayoutY();

            vertLayout.AddFlexibleSpace();
            GUILayoutX horzLayout = vertLayout.AddLayoutX();
            horzLayout.AddFlexibleSpace();
            GUILayout contentLayout = horzLayout.AddLayoutY();
            GUILayout timeLayout = contentLayout.AddLayoutX();
            timeLayout.AddSpace(5);
            timeLayout.AddElement(timeField);
            timeLayout.AddFlexibleSpace();
            GUILayout componentLayout = contentLayout.AddLayoutX();
            componentLayout.AddSpace(5);
            componentLayout.AddElement(componentField);
            componentLayout.AddFlexibleSpace();
            GUILayout methodLayout = contentLayout.AddLayoutX();
            methodLayout.AddSpace(5);
            methodLayout.AddElement(methodField);
            methodLayout.AddFlexibleSpace();
            horzLayout.AddFlexibleSpace();
            vertLayout.AddFlexibleSpace();
        }
    }

    /** @} */
}

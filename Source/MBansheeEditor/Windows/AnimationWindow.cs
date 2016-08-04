//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// Displays animation curve editor window.
    /// </summary>
    [DefaultSize(900, 500)]
    internal class AnimationWindow : EditorWindow
    {
        /// <summary>
        /// A set of animation curves for a field of a certain type.
        /// </summary>
        private struct FieldCurves
        {
            public SerializableProperty.FieldType type;
            public EdAnimationCurve[] curves;
        }

        private const int FIELD_DISPLAY_WIDTH = 200;
        private const int DRAG_START_DISTANCE = 3;
        private const float DRAG_SCALE = 10.0f;
        private const float ZOOM_SCALE = 15.0f;

        private bool isInitialized;
        private GUIButton playButton;
        private GUIButton recordButton;

        private GUIButton prevFrameButton;
        private GUIIntField frameInputField;
        private GUIButton nextFrameButton;

        private GUIButton addKeyframeButton;
        private GUIButton addEventButton;

        private GUIButton optionsButton;

        private GUIButton addPropertyBtn;
        private GUIButton delPropertyBtn;

        private GUILayout buttonLayout;

        private int buttonLayoutHeight;
        private int scrollBarWidth;
        private int scrollBarHeight;

        private GUIResizeableScrollBarH horzScrollBar;
        private GUIResizeableScrollBarV vertScrollBar;

        private GUIPanel editorPanel;
        private GUIAnimFieldDisplay guiFieldDisplay;
        private GUICurveEditor guiCurveEditor;

        private SceneObject selectedSO;
        private int currentFrameIdx;
        private int fps = 1;

        private Vector2I dragStartPos;
        private bool isButtonHeld;
        private bool isDragInProgress;

        private Vector2 minimalRange;
        private Vector2 visibleOffset;

        private Dictionary<string, FieldCurves> curves = new Dictionary<string, FieldCurves>();
        private List<string> selectedFields = new List<string>();

        internal int FPS
        {
            get { return fps; }
            set { guiCurveEditor.SetFPS(value); fps = MathEx.Max(value, 1); }
        }

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
            Selection.OnSelectionChanged += OnSelectionChanged;
            EditorInput.OnPointerPressed += OnPointerPressed;
            EditorInput.OnPointerMoved += OnPointerMoved;
            EditorInput.OnPointerReleased += OnPointerReleased;
            EditorInput.OnButtonUp += OnButtonUp;

            Rebuild();
        }

        private Vector2 GetVisibleRange()
        {
            float unitsPerXPixel = guiCurveEditor.XRange / guiCurveEditor.Width;
            float unitsPerYPixel = guiCurveEditor.YRange / guiCurveEditor.Height;

            Vector2I visibleSize = GetCurveEditorSize();
            return new Vector2(unitsPerXPixel * visibleSize.x, unitsPerYPixel * visibleSize.y);
        }

        private void SetVisibleOffset(Vector2 offset)
        {
            visibleOffset = offset;

            float pixelsPerXUnit = guiCurveEditor.Width / guiCurveEditor.XRange;
            float pixelsPerYUnit = guiCurveEditor.Height / (guiCurveEditor.YRange * 2.0f);

            int x = (int) (pixelsPerXUnit*visibleOffset.x);
            int y = (int) (pixelsPerYUnit*visibleOffset.y);

            guiCurveEditor.SetPosition(x, y);
        }

        // Increases range without zooming in (increasing width/height accordingly)
        private void SetTotalRange(float x, float y)
        {
            float pixelsPerXUnit = guiCurveEditor.Width / guiCurveEditor.XRange;
            float pixelsPerYUnit = guiCurveEditor.Height / (guiCurveEditor.YRange * 2.0f);

            int width = (int) (pixelsPerXUnit * x);
            int height = (int) (pixelsPerYUnit * y);

            guiCurveEditor.SetRange(x, y);
            guiCurveEditor.SetSize(width, height);

            UpdateScrollBarSize();
        }

        private void UpdateScrollBarSize()
        {
            Vector2 visibleRange = GetVisibleRange();
            Vector2 totalRange = new Vector2(guiCurveEditor.XRange, guiCurveEditor.YRange);

            horzScrollBar.HandleSize = visibleRange.x / totalRange.x;
            vertScrollBar.HandleSize = visibleRange.y / totalRange.y;
        }

        private void UpdateScrollBarPosition()
        {
            Vector2 visibleRange = GetVisibleRange();
            Vector2 totalRange = new Vector2(guiCurveEditor.XRange, guiCurveEditor.YRange);
            Vector2 scrollableRange = totalRange - visibleRange;

            horzScrollBar.Position = visibleOffset.x / scrollableRange.x;
            vertScrollBar.Position = visibleOffset.y / scrollableRange.y;
        }

        private void Zoom(Vector2 curvePos, float amount)
        {
            Vector2 relativePos = curvePos - visibleOffset;
            Vector2 visibleRange = GetVisibleRange();

            relativePos.x /= visibleRange.x;
            relativePos.y /= visibleRange.y;

            relativePos.x = relativePos.x * 2.0f - 1.0f;
            relativePos.y = relativePos.y * 2.0f - 1.0f;

            Vector2 offset = visibleOffset;
            offset.x += relativePos.x * amount;
            offset.y += relativePos.y * amount;

            offset.x = Math.Max(0.0f, offset.x);

            SetVisibleOffset(offset);
            UpdateScrollBarPosition();

            int width = guiCurveEditor.Width + (int)amount;
            int height = guiCurveEditor.Height + (int)amount;

            // If we aren't at the minimum size, modify size and offset
            Vector2I visibleSize = GetCurveEditorSize();
            if (width > visibleSize.x || height > visibleSize.y)
            {
                width = Math.Max(width, visibleSize.x);
                height = Math.Max(height, visibleSize.y);

                guiCurveEditor.SetSize(width, height);
                UpdateScrollBarSize();
            }
            else // Otherwise start increasing range for zoom in
            {
                float unitsPerXPixel = guiCurveEditor.XRange / guiCurveEditor.Width;
                float unitsPerYPixel = guiCurveEditor.YRange / guiCurveEditor.Height;

                float rangeX = guiCurveEditor.XRange + unitsPerXPixel * amount;
                float rangeY = guiCurveEditor.YRange + unitsPerYPixel * amount;

                SetTotalRange(rangeX, rangeY);
            }
        }

        private void OnEditorUpdate()
        {
            if (!isInitialized)
                return;

            // Handle middle mouse dragging
            if (isDragInProgress)
            {
                float dragX = Input.GetAxisValue(InputAxis.MouseX) * DRAG_SCALE;
                float dragY = Input.GetAxisValue(InputAxis.MouseY) * DRAG_SCALE;

                Vector2 totalRange = new Vector2(guiCurveEditor.XRange, guiCurveEditor.YRange);
                Vector2 visibleRange = GetVisibleRange();
                Vector2 offset = visibleOffset;

                if (dragX > 0.0f)
                {
                    offset.x += dragX;

                    float visibleRight = offset.x + visibleRange.x;
                    if (visibleRight > totalRange.x)
                        totalRange.x = visibleRight;
                }
                else
                {
                    float actualDragX = offset.x - Math.Max(0.0f, offset.x + dragX);

                    offset.x -= actualDragX;

                    float visibleRight = offset.x + visibleRange.x;
                    totalRange.x = Math.Max(minimalRange.x, visibleRight);
                }

                if (dragY > 0.0f)
                {
                    offset.y += dragY;

                    float visibleTop = offset.y + visibleRange.y;
                    if (visibleTop > totalRange.y)
                        totalRange.y = visibleTop;
                }
                else
                {
                    offset.y += dragY;

                    float visibleYMax = Math.Abs(offset.y) + visibleRange.y;
                    totalRange.y = Math.Max(minimalRange.y, visibleYMax);
                }

                SetTotalRange(totalRange.x, totalRange.y);
                SetVisibleOffset(offset);
                UpdateScrollBarPosition();
            }

            // Handle zoom in/out
            float scroll = Input.GetAxisValue(InputAxis.MouseZ);
            if (scroll != 0.0f)
            {
                Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);
                Vector2 curvePos;
                if (guiCurveEditor.WindowToCurveSpace(windowPos, out curvePos))
                {
                    float zoom = scroll*ZOOM_SCALE;
                    Zoom(curvePos, zoom);
                }
            }
        }

        private void OnDestroy()
        {
            Selection.OnSelectionChanged -= OnSelectionChanged;
            EditorInput.OnPointerPressed -= OnPointerPressed;
            EditorInput.OnPointerMoved -= OnPointerMoved;
            EditorInput.OnPointerReleased -= OnPointerReleased;
            EditorInput.OnButtonUp -= OnButtonUp;
        }

        protected override void WindowResized(int width, int height)
        {
            if (!isInitialized)
                return;

            guiFieldDisplay.SetSize(FIELD_DISPLAY_WIDTH, height - buttonLayoutHeight*2);

            Vector2I curveEditorSize = GetCurveEditorSize();
            guiCurveEditor.SetSize(curveEditorSize.x, curveEditorSize.y);
            guiCurveEditor.Redraw();

            horzScrollBar.SetWidth(curveEditorSize.x);
            vertScrollBar.SetHeight(curveEditorSize.y);

            UpdateScrollBarSize();
            UpdateScrollBarPosition();
        }

        private void Rebuild()
        {
            GUI.Clear();
            selectedFields.Clear();
            curves.Clear();
            isInitialized = false;

            selectedSO = Selection.SceneObject;
            if (selectedSO == null)
            {
                GUILabel warningLbl = new GUILabel(new LocEdString("Select an object to animate in the Hierarchy or Scene windows."));

                GUILayoutY vertLayout = GUI.AddLayoutY();
                vertLayout.AddFlexibleSpace();
                GUILayoutX horzLayout = vertLayout.AddLayoutX();
                vertLayout.AddFlexibleSpace();

                horzLayout.AddFlexibleSpace();
                horzLayout.AddElement(warningLbl);
                horzLayout.AddFlexibleSpace();
                
                return;
            }

            // TODO - Retrieve Animation & AnimationClip from the selected object, fill curves dictionary
            //  - If not available, show a button to create new animation clip

            // Top button row
            GUIContent playIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.Play),
                new LocEdString("Play"));
            GUIContent recordIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.Record),
                new LocEdString("Record"));

            GUIContent prevFrameIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.FrameBack),
                new LocEdString("Previous frame"));
            GUIContent nextFrameIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.FrameForward),
                new LocEdString("Next frame"));

            GUIContent addKeyframeIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.AddKeyframe),
                new LocEdString("Add keyframe"));
            GUIContent addEventIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.AddEvent),
                new LocEdString("Add event"));

            GUIContent optionsIcon = new GUIContent(EditorBuiltin.GetLibraryWindowIcon(LibraryWindowIcon.Options),
                new LocEdString("Options"));

            playButton = new GUIButton(playIcon);
            recordButton = new GUIButton(recordIcon);

            prevFrameButton = new GUIButton(prevFrameIcon);
            frameInputField = new GUIIntField();
            nextFrameButton = new GUIButton(nextFrameIcon);

            addKeyframeButton = new GUIButton(addKeyframeIcon);
            addEventButton = new GUIButton(addEventIcon);

            optionsButton = new GUIButton(optionsIcon);

            playButton.OnClick += () =>
            {
                // TODO
                // - Record current state of the scene object hierarchy
                // - Evaluate all curves manually and update them
                // - On end, restore original values of the scene object hierarchy
            };

            recordButton.OnClick += () =>
            {
                // TODO
                // - Every frame read back current values of all the current curve's properties and assign it to the current frame
            };

            prevFrameButton.OnClick += () =>
            {
                SetCurrentFrame(currentFrameIdx - 1);
            };

            frameInputField.OnChanged += SetCurrentFrame;

            nextFrameButton.OnClick += () =>
            {
                SetCurrentFrame(currentFrameIdx + 1);
            };

            addKeyframeButton.OnClick += () =>
            {
                guiCurveEditor.AddKeyFrameAtMarker();

                // TODO - Update local curves?
            };

            addEventButton.OnClick += () =>
            {
                // TODO - Add event
            };

            optionsButton.OnClick += () =>
            {
                Vector2I openPosition = ScreenToWindowPos(Input.PointerPosition);
                AnimationOptions dropDown = DropDownWindow.Open<AnimationOptions>(this, openPosition);
                dropDown.Initialize(this);
            };

            // Property buttons
            addPropertyBtn = new GUIButton(new LocEdString("Add property"));
            delPropertyBtn = new GUIButton(new LocEdString("Delete selected"));

            addPropertyBtn.OnClick += () =>
            {
                Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);
                FieldSelectionWindow fieldSelection = DropDownWindow.Open<FieldSelectionWindow>(this, windowPos);
                fieldSelection.OnFieldSelected += OnFieldAdded;
            };

            delPropertyBtn.OnClick += () =>
            {
                LocEdString title = new LocEdString("Warning");
                LocEdString message = new LocEdString("Are you sure you want to remove all selected fields?");

                DialogBox.Open(title, message, DialogBox.Type.YesNo, x =>
                {
                    if (x == DialogBox.ResultType.Yes)
                    {
                        RemoveSelectedFields();
                    }
                });
            };

            GUILayout mainLayout = GUI.AddLayoutY();

            buttonLayout = mainLayout.AddLayoutX();
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(playButton);
            buttonLayout.AddElement(recordButton);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(prevFrameButton);
            buttonLayout.AddElement(frameInputField);
            buttonLayout.AddElement(nextFrameButton);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(addKeyframeButton);
            buttonLayout.AddElement(addEventButton);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(optionsButton);
            buttonLayout.AddFlexibleSpace();

            buttonLayoutHeight = playButton.Bounds.height;
            
            GUILayout contentLayout = mainLayout.AddLayoutX();
            GUILayout fieldDisplayLayout = contentLayout.AddLayoutY(GUIOption.FixedWidth(FIELD_DISPLAY_WIDTH));

            guiFieldDisplay = new GUIAnimFieldDisplay(fieldDisplayLayout, FIELD_DISPLAY_WIDTH,
                Height - buttonLayoutHeight * 2, selectedSO);
            guiFieldDisplay.OnEntrySelected += OnFieldSelected;

            GUILayout bottomButtonLayout = fieldDisplayLayout.AddLayoutX();
            bottomButtonLayout.AddElement(addPropertyBtn);
            bottomButtonLayout.AddElement(delPropertyBtn);

            horzScrollBar = new GUIResizeableScrollBarH();
            horzScrollBar.OnScrollOrResize += OnHorzScrollOrResize;
            
            vertScrollBar = new GUIResizeableScrollBarV();
            vertScrollBar.OnScrollOrResize += OnVertScrollOrResize;
            
            GUILayout curveLayout = contentLayout.AddLayoutY();
            GUILayout curveLayoutHorz = curveLayout.AddLayoutX();
            GUILayout horzScrollBarLayout = curveLayout.AddLayoutX();
            horzScrollBarLayout.AddElement(horzScrollBar);
            horzScrollBarLayout.AddFlexibleSpace();

            editorPanel = curveLayoutHorz.AddPanel();
            curveLayoutHorz.AddElement(vertScrollBar);
            curveLayoutHorz.AddFlexibleSpace();

            scrollBarHeight = horzScrollBar.Bounds.height;
            scrollBarWidth = vertScrollBar.Bounds.width;

            Vector2I curveEditorSize = GetCurveEditorSize();
            guiCurveEditor = new GUICurveEditor(this, editorPanel, curveEditorSize.x, curveEditorSize.y);
            guiCurveEditor.OnFrameSelected += OnFrameSelected;
            guiCurveEditor.Redraw();

            horzScrollBar.SetWidth(curveEditorSize.x);
            vertScrollBar.SetHeight(curveEditorSize.y);

            SetCurrentFrame(currentFrameIdx);
            UpdateScrollBarSize();

            visibleOffset = new Vector2(0.0f, 0.0f);
            isInitialized = true;
        }

        private void SetCurrentFrame(int frameIdx)
        {
            currentFrameIdx = Math.Max(0, frameIdx);

            frameInputField.Value = currentFrameIdx;
            guiCurveEditor.SetMarkedFrame(currentFrameIdx);

            float time = guiCurveEditor.GetTimeForFrame(currentFrameIdx);

            List<GUIAnimFieldPathValue> values = new List<GUIAnimFieldPathValue>();
            foreach (var kvp in curves)
            {
                SerializableProperty property = GUIAnimFieldDisplay.FindProperty(selectedSO, kvp.Key);
                if (property != null)
                {
                    GUIAnimFieldPathValue fieldValue = new GUIAnimFieldPathValue();
                    fieldValue.path = kvp.Key;

                    switch (kvp.Value.type)
                    {
                        case SerializableProperty.FieldType.Vector2:
                        {
                            Vector2 value = new Vector2();

                            for(int i = 0; i < 2; i++)
                                value[i] = kvp.Value.curves[i].Evaluate(time, false);

                            fieldValue.value = value;
                        }
                            break;
                        case SerializableProperty.FieldType.Vector3:
                            {
                                Vector3 value = new Vector3();

                                for (int i = 0; i < 3; i++)
                                    value[i] = kvp.Value.curves[i].Evaluate(time, false);

                                fieldValue.value = value;
                            }
                            break;
                        case SerializableProperty.FieldType.Vector4:
                            {
                                Vector4 value = new Vector4();

                                for (int i = 0; i < 4; i++)
                                    value[i] = kvp.Value.curves[i].Evaluate(time, false);

                                fieldValue.value = value;
                            }
                            break;
                        case SerializableProperty.FieldType.Color:
                            {
                                Color value = new Color();

                                for (int i = 0; i < 4; i++)
                                    value[i] = kvp.Value.curves[i].Evaluate(time, false);

                                fieldValue.value = value;
                            }
                            break;
                        case SerializableProperty.FieldType.Bool:
                        case SerializableProperty.FieldType.Int:
                        case SerializableProperty.FieldType.Float:
                            fieldValue.value = kvp.Value.curves[0].Evaluate(time, false); ;
                            break;
                    }

                    values.Add(fieldValue);
                }
            }

            guiFieldDisplay.SetDisplayValues(values.ToArray());
        }

        private void OnPointerPressed(PointerEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerPressed(ev);

            if (ev.button == PointerButton.Middle)
            {
                Vector2I windowPos = ScreenToWindowPos(ev.ScreenPos);
                Vector2 curvePos;
                if (guiCurveEditor.WindowToCurveSpace(windowPos, out curvePos))
                {
                    dragStartPos = windowPos;
                    isButtonHeld = true;
                }
            }
        }

        private void OnPointerMoved(PointerEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerMoved(ev);

            if (isButtonHeld)
            {
                Vector2I windowPos = ScreenToWindowPos(ev.ScreenPos);

                int distance = Vector2I.Distance(dragStartPos, windowPos);
                if (distance >= DRAG_START_DISTANCE)
                {
                    isDragInProgress = true;

                    Cursor.Hide();

                    Rect2I clipRect;
                    clipRect.x = ev.ScreenPos.x - 2;
                    clipRect.y = ev.ScreenPos.y - 2;
                    clipRect.width = 4;
                    clipRect.height = 4;

                    Cursor.ClipToRect(clipRect);
                }
            }
        }

        private void OnPointerReleased(PointerEvent ev)
        {
            if (isDragInProgress)
            {
                Cursor.Show();
                Cursor.ClipDisable();
            }

            isButtonHeld = false;
            isDragInProgress = false;

            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerReleased(ev);
        }

        private void OnButtonUp(ButtonEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnButtonUp(ev);
        }

        private void UpdateDisplayedCurves()
        {
            List<EdAnimationCurve> curvesToDisplay = new List<EdAnimationCurve>();
            for (int i = 0; i < selectedFields.Count; i++)
            {
                EdAnimationCurve curve;
                if(TryGetCurve(selectedFields[i], out curve))
                    curvesToDisplay.Add(curve);
            }

            guiCurveEditor.SetCurves(curvesToDisplay.ToArray());

            float xRange;
            float yRange;
            CalculateRange(curvesToDisplay, out xRange, out yRange);

            // Don't allow zero range
            if (xRange == 0.0f)
                xRange = 60.0f;

            if (yRange == 0.0f)
                yRange = 10.0f;

            // Add padding to y range
            yRange *= 1.05f;

            // Don't reduce visible range
            minimalRange.x = Math.Max(xRange, minimalRange.x);
            minimalRange.y = Math.Max(yRange, minimalRange.y);

            xRange = Math.Max(xRange, guiCurveEditor.XRange);
            yRange = Math.Max(yRange, guiCurveEditor.YRange);

            guiCurveEditor.SetRange(xRange, yRange);
            UpdateScrollBarSize();
        }

        private Vector2I GetCurveEditorSize()
        {
            Vector2I output = new Vector2I();
            output.x = Math.Max(0, Width - FIELD_DISPLAY_WIDTH - scrollBarWidth);
            output.y = Math.Max(0, Height - buttonLayoutHeight - scrollBarHeight);

            return output;
        }

        private static void CalculateRange(List<EdAnimationCurve> curves, out float xRange, out float yRange)
        {
            xRange = 0.0f;
            yRange = 0.0f;

            foreach (var curve in curves)
            {
                KeyFrame[] keyframes = curve.KeyFrames;

                foreach (var key in keyframes)
                {
                    xRange = Math.Max(xRange, key.time);
                    yRange = Math.Max(yRange, Math.Abs(key.value));
                }
            }
        }

        private bool TryGetCurve(string path, out EdAnimationCurve curve)
        {
            int index = path.LastIndexOf(".");
            string parentPath;
            string subPathSuffix = null;
            if (index == -1)
            {
                parentPath = path;
            }
            else
            {
                parentPath = path.Substring(0, index);
                subPathSuffix = path.Substring(index, path.Length - index);
            }

            FieldCurves fieldCurves;
            if (curves.TryGetValue(parentPath, out fieldCurves))
            {
                if (!string.IsNullOrEmpty(subPathSuffix))
                {
                    if (subPathSuffix == ".x" || subPathSuffix == ".r")
                    {
                        curve = fieldCurves.curves[0];
                        return true;
                    }
                    else if (subPathSuffix == ".y" || subPathSuffix == ".g")
                    {
                        curve = fieldCurves.curves[1];
                        return true;
                    }
                    else if (subPathSuffix == ".z" || subPathSuffix == ".b")
                    {
                        curve = fieldCurves.curves[2];
                        return true;
                    }
                    else if (subPathSuffix == ".w" || subPathSuffix == ".a")
                    {
                        curve = fieldCurves.curves[3];
                        return true;
                    }
                }
                else
                {
                    curve = fieldCurves.curves[0];
                    return true;
                }
            }

            curve = null;
            return false;
        }

        private void OnFieldAdded(string path, SerializableProperty.FieldType type)
        {
            guiFieldDisplay.AddField(path);

            switch (type)
            {
                case SerializableProperty.FieldType.Vector4:
                {
                    FieldCurves fieldCurves = new FieldCurves();
                    fieldCurves.type = type;
                    fieldCurves.curves = new EdAnimationCurve[4];

                    string[] subPaths = { ".x", ".y", ".z", ".w" };
                    for (int i = 0; i < subPaths.Length; i++)
                    {
                        string subFieldPath = path + subPaths[i];
                        fieldCurves.curves[i] = new EdAnimationCurve();
                        selectedFields.Add(subFieldPath);
                    }

                    curves[path] = fieldCurves;
                }
                    break;
                case SerializableProperty.FieldType.Vector3:
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[3];

                        string[] subPaths = { ".x", ".y", ".z" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curves[i] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        curves[path] = fieldCurves;
                    }
                    break;
                case SerializableProperty.FieldType.Vector2:
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[2];

                        string[] subPaths = { ".x", ".y" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curves[i] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        curves[path] = fieldCurves;
                    }
                    break;
                case SerializableProperty.FieldType.Color:
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[4];

                        string[] subPaths = { ".r", ".g", ".b", ".a" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curves[i] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        curves[path] = fieldCurves;
                    }
                    break;
                default: // Primitive type
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[1];

                        fieldCurves.curves[0] = new EdAnimationCurve();
                        selectedFields.Add(path);

                        curves[path] = fieldCurves;
                    }
                    break;
            }

            UpdateDisplayedCurves();
        }

        private bool IsPathParent(string child, string parent)
        {
            string[] childEntries = child.Split('/', '.');
            string[] parentEntries = parent.Split('/', '.');

            if (parentEntries.Length >= child.Length)
                return false;

            int compareLength = Math.Min(childEntries.Length, parentEntries.Length);
            for (int i = 0; i < compareLength; i++)
            {
                if (childEntries[i] != parentEntries[i])
                    return false;
            }

            return true;
        }

        private string GetSubPathParent(string path)
        {
            int index = path.LastIndexOf(".");
            if (index == -1)
                return path;

            return path.Substring(0, index);
        }

        private void OnHorzScrollOrResize(float position, float size)
        {
            Vector2 visibleRange = GetVisibleRange();
            float scrollableRange = guiCurveEditor.XRange - visibleRange.x;

            Vector2 offset = visibleOffset;
            offset.x = scrollableRange * position;

            SetVisibleOffset(offset);

            int width = (int)(guiCurveEditor.Width / size);
            guiCurveEditor.SetSize(width, guiCurveEditor.Height);
        }

        private void OnVertScrollOrResize(float position, float size)
        {
            Vector2 visibleRange = GetVisibleRange();
            float scrollableRange = guiCurveEditor.YRange - visibleRange.y;

            Vector2 offset = visibleOffset;
            offset.y = scrollableRange*position;

            SetVisibleOffset(offset);

            int height = (int)(guiCurveEditor.Height / size);
            guiCurveEditor.SetSize(guiCurveEditor.Width, height);
        }

        private void OnFieldSelected(string path)
        {
            if (!Input.IsButtonHeld(ButtonCode.LeftShift) && !Input.IsButtonHeld(ButtonCode.RightShift))
                selectedFields.Clear();

            if (!string.IsNullOrEmpty(path))
            {
                selectedFields.RemoveAll(x => { return x == path || IsPathParent(x, path); });
                selectedFields.Add(path);
            }

            guiFieldDisplay.SetSelection(selectedFields.ToArray());

            UpdateDisplayedCurves();
        }

        private void RemoveSelectedFields()
        {
            for (int i = 0; i < selectedFields.Count; i++)
            {
                selectedFields.Remove(selectedFields[i]);
                curves.Remove(GetSubPathParent(selectedFields[i]));
            }

            List<string> existingFields = new List<string>();
            foreach(var KVP in curves)
                existingFields.Add(KVP.Key);

            guiFieldDisplay.SetFields(existingFields.ToArray());

            selectedFields.Clear();
            UpdateDisplayedCurves();
        }

        private void OnSelectionChanged(SceneObject[] sceneObjects, string[] resourcePaths)
        {
            Rebuild();
        }

        private void OnFrameSelected(int frameIdx)
        {
            SetCurrentFrame(frameIdx);
        }
    }

    /// <summary>
    /// Drop down window that displays options used by the animation window.
    /// </summary>
    [DefaultSize(100, 50)]
    internal class AnimationOptions : DropDownWindow
    {
        /// <summary>
        /// Initializes the drop down window by creating the necessary GUI. Must be called after construction and before
        /// use.
        /// </summary>
        /// <param name="parent">Animation window that this drop down window is a part of.</param>
        internal void Initialize(AnimationWindow parent)
        {
            GUIIntField fpsField = new GUIIntField(new LocEdString("FPS"), 40);
            fpsField.Value = parent.FPS;
            fpsField.OnChanged += x => { parent.FPS = x; };
            
            GUILayoutY vertLayout = GUI.AddLayoutY();

            vertLayout.AddFlexibleSpace();
            GUILayoutX contentLayout = vertLayout.AddLayoutX();
            contentLayout.AddFlexibleSpace();
            contentLayout.AddElement(fpsField);
            contentLayout.AddFlexibleSpace();
            vertLayout.AddFlexibleSpace();
        }
    }

    /** @} */
}

//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// Displays animation curve editor window. Allows the user to manipulate keyframes of animation curves, add/remove
    /// curves from an animation clip, and manipulate animation events.
    /// </summary>
    [DefaultSize(900, 500)]
    internal class AnimationWindow : EditorWindow
    {
        private const int FIELD_DISPLAY_WIDTH = 300;
        private const int DRAG_START_DISTANCE = 3;
        private const float DRAG_SCALE = 3.0f;
        private const float ZOOM_SCALE = 0.1f/120.0f; // One scroll step is usually 120 units, we want 1/10 of that

        private SceneObject selectedSO;

        /// <summary>
        /// Scene object for which are we currently changing the animation for.
        /// </summary>
        internal SceneObject SelectedSO
        {
            get { return selectedSO; }
        }

        #region Overrides

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

            UpdateSelectedSO(true);
        }

        private void OnEditorUpdate()
        {
            if (selectedSO == null)
                return;

            HandleDragAndZoomInput();

            if (state == State.Playback)
            {
                Animation animation = selectedSO.GetComponent<Animation>();
                if (animation != null)
                {
                    float time = animation.EditorGetTime();
                    int frameIdx = (int)(time * fps);

                    SetCurrentFrame(frameIdx);

                    animation.UpdateFloatProperties();
                }
            }
            else if (state == State.Recording)
            {
                if (!delayRecord)
                {
                    float time = guiCurveEditor.GetTimeForFrame(currentFrameIdx);
                    if (RecordState(time))
                    {
                        ApplyClipChanges();
                        guiCurveEditor.Redraw();
                    }
                }
            }

            delayRecord = false;
        }

        private void OnDestroy()
        {
            SwitchState(State.Empty);

            Selection.OnSelectionChanged -= OnSelectionChanged;

            if (selectedSO != null)
            {
                EditorInput.OnPointerPressed -= OnPointerPressed;
                EditorInput.OnPointerMoved -= OnPointerMoved;
                EditorInput.OnPointerReleased -= OnPointerReleased;
                EditorInput.OnButtonUp -= OnButtonUp;
            }
        }

        /// <inheritdoc/>
        protected override void WindowResized(int width, int height)
        {
            if (selectedSO == null)
                return;

            ResizeGUI(width, height);
        }
        #endregion

        #region GUI
        private GUIToggle playButton;
        private GUIToggle recordButton;

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

        /// <summary>
        /// Recreates the entire curve editor GUI depending on the currently selected scene object.
        /// </summary>
        private void RebuildGUI()
        {
            GUI.Clear();
            guiCurveEditor = null;
            guiFieldDisplay = null;

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

            playButton = new GUIToggle(playIcon, EditorStyles.Button);
            recordButton = new GUIToggle(recordIcon, EditorStyles.Button);

            prevFrameButton = new GUIButton(prevFrameIcon);
            frameInputField = new GUIIntField();
            nextFrameButton = new GUIButton(nextFrameIcon);

            addKeyframeButton = new GUIButton(addKeyframeIcon);
            addEventButton = new GUIButton(addEventIcon);

            optionsButton = new GUIButton(optionsIcon);

            playButton.OnToggled += x =>
            {
                if(x)
                    SwitchState(State.Playback);
                else
                    SwitchState(State.Normal);
            };

            recordButton.OnToggled += x =>
            {
                if (x)
                    SwitchState(State.Recording);
                else
                    SwitchState(State.Normal);
            };

            prevFrameButton.OnClick += () =>
            {
                SetCurrentFrame(currentFrameIdx - 1);

                switch (state)
                {
                    case State.Recording:
                    case State.Normal:
                        PreviewFrame(currentFrameIdx);
                        break;
                    default:
                        SwitchState(State.Normal);
                        break;
                }
            };

            frameInputField.OnChanged += x =>
            {
                SetCurrentFrame(x);

                switch (state)
                {
                    case State.Recording:
                    case State.Normal:
                        PreviewFrame(currentFrameIdx);
                        break;
                    default:
                        SwitchState(State.Normal);
                        break;
                }
            };

            nextFrameButton.OnClick += () =>
            {
                SetCurrentFrame(currentFrameIdx + 1);

                switch (state)
                {
                    case State.Recording:
                    case State.Normal:
                        PreviewFrame(currentFrameIdx);
                        break;
                    default:
                        SwitchState(State.Normal);
                        break;
                }
            };

            addKeyframeButton.OnClick += () =>
            {
                SwitchState(State.Normal);
                guiCurveEditor.AddKeyFrameAtMarker();
            };

            addEventButton.OnClick += () =>
            {
                SwitchState(State.Normal);
                guiCurveEditor.AddEventAtMarker();
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
                Action openPropertyWindow = () =>
                {
                    Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);
                    FieldSelectionWindow fieldSelection = DropDownWindow.Open<FieldSelectionWindow>(this, windowPos);
                    fieldSelection.OnFieldSelected += OnFieldAdded;
                };

                if (clipInfo.clip == null)
                {
                    LocEdString title = new LocEdString("Warning");
                    LocEdString message =
                        new LocEdString("Selected object doesn't have an animation clip assigned. Would you like to create" +
                                        " a new animation clip?");

                    DialogBox.Open(title, message, DialogBox.Type.YesNoCancel, type =>
                    {
                        if (type == DialogBox.ResultType.Yes)
                        {
                            string clipSavePath;
                            if (BrowseDialog.SaveFile(ProjectLibrary.ResourceFolder, "*.asset", out clipSavePath))
                            {
                                SwitchState(State.Empty);
                                clipSavePath = Path.ChangeExtension(clipSavePath, ".asset");

                                AnimationClip newClip = new AnimationClip();

                                ProjectLibrary.Create(newClip, clipSavePath);
                                LoadAnimClip(newClip);

                                Animation animation = selectedSO.GetComponent<Animation>();
                                if (animation == null)
                                    animation = selectedSO.AddComponent<Animation>();

                                animation.DefaultClip = newClip;
                                EditorApplication.SetSceneDirty();

                                SwitchState(State.Normal);
                                openPropertyWindow();
                            }
                        }
                    });
                }
                else
                {
                    if (clipInfo.isImported)
                    {
                        LocEdString title = new LocEdString("Warning");
                        LocEdString message =
                            new LocEdString("You cannot add/edit/remove curves from animation clips that" +
                                            " are imported from an external file.");

                        DialogBox.Open(title, message, DialogBox.Type.OK);
                    }
                    else
                    {
                        SwitchState(State.Normal);
                        openPropertyWindow();
                    }
                }
            };

            delPropertyBtn.OnClick += () =>
            {
                if (clipInfo.clip == null)
                    return;

                SwitchState(State.Normal);

                if (clipInfo.isImported)
                {
                    LocEdString title = new LocEdString("Warning");
                    LocEdString message =
                        new LocEdString("You cannot add/edit/remove curves from animation clips that" +
                                        " are imported from an external file.");

                    DialogBox.Open(title, message, DialogBox.Type.OK);
                }
                else
                {
                    LocEdString title = new LocEdString("Warning");
                    LocEdString message = new LocEdString("Are you sure you want to remove all selected fields?");

                    DialogBox.Open(title, message, DialogBox.Type.YesNo, x =>
                    {
                        if (x == DialogBox.ResultType.Yes)
                        {
                            RemoveSelectedFields();
                            ApplyClipChanges();
                        }
                    });
                }
            };

            GUIPanel mainPanel = GUI.AddPanel();
            GUIPanel backgroundPanel = GUI.AddPanel(1);

            GUILayout mainLayout = mainPanel.AddLayoutY();

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

            GUITexture buttonBackground = new GUITexture(null, EditorStyles.HeaderBackground);
            buttonBackground.Bounds = new Rect2I(0, 0, Width, buttonLayoutHeight);
            backgroundPanel.AddElement(buttonBackground);

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

            GUITexture separator = new GUITexture(null, EditorStyles.Separator, GUIOption.FixedWidth(3));
            contentLayout.AddElement(separator);

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
            guiCurveEditor.OnEventAdded += OnEventsChanged;
            guiCurveEditor.OnEventModified += EditorApplication.SetProjectDirty;
            guiCurveEditor.OnEventDeleted += OnEventsChanged;
            guiCurveEditor.OnCurveModified += () =>
            {
                SwitchState(State.Normal);

                ApplyClipChanges();
                PreviewFrame(currentFrameIdx);

                EditorApplication.SetProjectDirty();
            };
            guiCurveEditor.OnClicked += () =>
            {
                if(state != State.Recording)
                    SwitchState(State.Normal);
            };
            guiCurveEditor.Redraw();

            horzScrollBar.SetWidth(curveEditorSize.x);
            vertScrollBar.SetHeight(curveEditorSize.y);

            UpdateScrollBarSize();
        }

        /// <summary>
        /// Resizes GUI elements so they fit within the provided boundaries.
        /// </summary>
        /// <param name="width">Width of the GUI bounds, in pixels.</param>
        /// <param name="height">Height of the GUI bounds, in pixels.</param>
        private void ResizeGUI(int width, int height)
        {
            guiFieldDisplay.SetSize(FIELD_DISPLAY_WIDTH, height - buttonLayoutHeight * 2);

            Vector2I curveEditorSize = GetCurveEditorSize();
            guiCurveEditor.SetSize(curveEditorSize.x, curveEditorSize.y);
            guiCurveEditor.Redraw();

            horzScrollBar.SetWidth(curveEditorSize.x);
            vertScrollBar.SetHeight(curveEditorSize.y);

            UpdateScrollBarSize();
            UpdateScrollBarPosition();
        }
        #endregion

        #region Scroll, drag, zoom
        private Vector2I dragStartPos;
        private bool isButtonHeld;
        private bool isDragInProgress;

        private float zoomAmount;

        /// <summary>
        /// Handles mouse scroll wheel and dragging events in order to zoom or drag the displayed curve editor contents.
        /// </summary>
        private void HandleDragAndZoomInput()
        {
            // Handle middle mouse dragging
            if (isDragInProgress)
            {
                float lengthPerPixel = guiCurveEditor.Range.x / guiCurveEditor.Width;
                float heightPerPixel = guiCurveEditor.Range.y / guiCurveEditor.Height;

                float dragX = Input.GetAxisValue(InputAxis.MouseX) * DRAG_SCALE * lengthPerPixel;
                float dragY = Input.GetAxisValue(InputAxis.MouseY) * DRAG_SCALE * heightPerPixel;

                Vector2 offset = guiCurveEditor.Offset;
                offset.x = Math.Max(0.0f, offset.x + dragX);
                offset.y -= dragY;

                guiCurveEditor.Offset = offset;
                UpdateScrollBarSize();
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
                    float zoom = scroll * ZOOM_SCALE;
                    Zoom(curvePos, zoom);
                }
            }
        }

        /// <summary>
        /// Moves or resizes the vertical scroll bar under the curve editor.
        /// </summary>
        /// <param name="position">New position of the scrollbar, in range [0, 1].</param>
        /// <param name="size">New size of the scrollbar handle, in range [0, 1].</param>
        private void SetVertScrollbarProperties(float position, float size)
        {
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 totalRange = GetTotalRange();

            visibleRange.y = totalRange.y*size;
            guiCurveEditor.Range = visibleRange;

            float scrollableRange = totalRange.y - visibleRange.y;

            Vector2 offset = guiCurveEditor.Offset;
            offset.y = -scrollableRange * (position * 2.0f - 1.0f);

            guiCurveEditor.Offset = offset;
        }

        /// <summary>
        /// Moves or resizes the horizontal scroll bar under the curve editor.
        /// </summary>
        /// <param name="position">New position of the scrollbar, in range [0, 1].</param>
        /// <param name="size">New size of the scrollbar handle, in range [0, 1].</param>
        private void SetHorzScrollbarProperties(float position, float size)
        {
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 totalRange = GetTotalRange();

            visibleRange.x = totalRange.x * size;
            guiCurveEditor.Range = visibleRange;

            float scrollableRange = totalRange.x - visibleRange.x;

            Vector2 offset = guiCurveEditor.Offset;
            offset.x = scrollableRange * position;

            guiCurveEditor.Offset = offset;
        }

        /// <summary>
        /// Updates the size of both scrollbars depending on the currently visible curve area vs. the total curve area.
        /// </summary>
        private void UpdateScrollBarSize()
        {
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 totalRange = GetTotalRange();

            horzScrollBar.HandleSize = visibleRange.x / totalRange.x;
            vertScrollBar.HandleSize = visibleRange.y / totalRange.y;
        }

        /// <summary>
        /// Updates the position of both scrollbars depending on the offset currently applied to the visible curve area.
        /// </summary>
        private void UpdateScrollBarPosition()
        {
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 totalRange = GetTotalRange();
            Vector2 scrollableRange = totalRange - visibleRange;

            Vector2 offset = guiCurveEditor.Offset;
            if (scrollableRange.x > 0.0f)
                horzScrollBar.Position = offset.x / scrollableRange.x;
            else
                horzScrollBar.Position = 0.0f;

            if (scrollableRange.y > 0.0f)
            {
                float pos = offset.y/scrollableRange.y;
                float sign = MathEx.Sign(pos);
                pos = sign*MathEx.Clamp01(MathEx.Abs(pos));
                pos = (1.0f - pos) /2.0f;

                vertScrollBar.Position = pos;
            }
            else
                vertScrollBar.Position = 0.0f;
        }

        /// <summary>
        /// Calculates the width/height of the curve area depending on the current zoom level.
        /// </summary>
        /// <returns>Width/height of the curve area, in curve space (value, time).</returns>
        private Vector2 GetZoomedRange()
        {
            float zoomLevel = MathEx.Pow(2, zoomAmount);

            Vector2 optimalRange = GetOptimalRange();
            return optimalRange / zoomLevel;
        }

        /// <summary>
        /// Returns the total width/height of the contents of the curve area.
        /// </summary>
        /// <returns>Width/height of the curve area, in curve space (value, time).</returns>
        private Vector2 GetTotalRange()
        {
            // Return optimal range (that covers the visible curve)
            Vector2 optimalRange = GetOptimalRange();

            // Increase range in case user zoomed out
            Vector2 zoomedRange = GetZoomedRange();
            return Vector2.Max(optimalRange, zoomedRange);
        }

        /// <summary>
        /// Zooms in or out at the provided position in the curve display.
        /// </summary>
        /// <param name="curvePos">Position to zoom towards, relative to the curve display area, in curve space 
        ///                        (value, time)</param>
        /// <param name="amount">Amount to zoom in (positive), or out (negative).</param>
        private void Zoom(Vector2 curvePos, float amount)
        {
            // Increase or decrease the visible range depending on zoom level
            Vector2 oldZoomedRange = GetZoomedRange();
            zoomAmount = MathEx.Clamp(zoomAmount + amount, -10.0f, 10.0f);
            Vector2 zoomedRange = GetZoomedRange();

            Vector2 zoomedDiff = zoomedRange - oldZoomedRange;

            Vector2 currentRange = guiCurveEditor.Range;
            Vector2 newRange = currentRange + zoomedDiff;
            guiCurveEditor.Range = newRange;

            // When zooming, make sure to focus on the point provided, so adjust the offset
            Vector2 rangeScale = newRange;
            rangeScale.x /= currentRange.x;
            rangeScale.y /= currentRange.y;

            Vector2 relativeCurvePos = curvePos - guiCurveEditor.Offset;
            Vector2 newCurvePos = relativeCurvePos * rangeScale;
            Vector2 diff = newCurvePos - relativeCurvePos;

            guiCurveEditor.Offset -= diff;

            UpdateScrollBarSize();
            UpdateScrollBarPosition();
        }
        #endregion

        #region Curve save/load
        private EditorAnimClipInfo clipInfo;

        /// <summary>
        /// Refreshes the contents of the curve and property display by loading animation curves from the provided
        /// animation clip.
        /// </summary>
        /// <param name="clip">Clip containing the animation to load.</param>
        private void LoadAnimClip(AnimationClip clip)
        {
            EditorPersistentData persistentData = EditorApplication.PersistentData;

            if (persistentData.dirtyAnimClips.TryGetValue(clip.UUID, out clipInfo))
            {
                // If an animation clip is imported, we don't care about it's cached curve values as they could have changed
                // since last modification, so we re-load the clip. But we persist the events as those can only be set
                // within the editor.
                if (clipInfo.isImported)
                {
                    EditorAnimClipInfo newClipInfo = EditorAnimClipInfo.Create(clip);
                    newClipInfo.events = clipInfo.events;
                }
            }
            else
                clipInfo = EditorAnimClipInfo.Create(clip);

            persistentData.dirtyAnimClips[clip.UUID] = clipInfo;

            foreach (var curve in clipInfo.curves)
                guiFieldDisplay.AddField(new AnimFieldInfo(curve.Key, curve.Value, !clipInfo.isImported));

            guiCurveEditor.Events = clipInfo.events;
            guiCurveEditor.DisableCurveEdit = clipInfo.isImported;

            SetCurrentFrame(0);
            FPS = clipInfo.sampleRate;
        }

        /// <summary>
        /// Applies any changes made to the animation curves and events to the actual animation clip resource.
        /// </summary>
        private void ApplyClipChanges()
        {
            if (clipInfo == null)
                return;

            EditorAnimClipTangents unused;
            clipInfo.Apply(out unused);
        }

        /// <summary>
        /// Checks if the currently selected object has changed, and rebuilds the GUI and loads the animation clip if needed.
        /// </summary>
        /// <param name="force">If true the GUI rebuild and animation clip load will be forced regardless if the active
        ///                     scene object changed.</param>
        private void UpdateSelectedSO(bool force)
        {
            SceneObject so = Selection.SceneObject;
            if (selectedSO != so || force)
            {
                if (selectedSO != null && so == null)
                {
                    EditorInput.OnPointerPressed -= OnPointerPressed;
                    EditorInput.OnPointerMoved -= OnPointerMoved;
                    EditorInput.OnPointerReleased -= OnPointerReleased;
                    EditorInput.OnButtonUp -= OnButtonUp;
                }
                else if (selectedSO == null && so != null)
                {
                    EditorInput.OnPointerPressed += OnPointerPressed;
                    EditorInput.OnPointerMoved += OnPointerMoved;
                    EditorInput.OnPointerReleased += OnPointerReleased;
                    EditorInput.OnButtonUp += OnButtonUp;
                }

                SwitchState(State.Empty);

                selectedSO = so;
                zoomAmount = 0.0f;
                selectedFields.Clear();
                clipInfo = null;

                RebuildGUI();

                // Load existing clip if one exists
                if (selectedSO != null)
                {
                    Animation animation = selectedSO.GetComponent<Animation>();
                    if (animation != null)
                    {
                        AnimationClip clip = animation.DefaultClip;
                        if (clip != null)
                            LoadAnimClip(clip);
                    }
                }

                if(clipInfo == null)
                    clipInfo = new EditorAnimClipInfo();

                SwitchState(State.Normal);

                if (selectedSO != null)
                    UpdateDisplayedCurves(true);
            }
        }

        /// <summary>
        /// Initializes the currently selected scene object. This should be called whenever the selected scene object 
        /// changes.
        /// </summary>
        private void InitializeSO()
        {
            if (selectedSO != null)
                soState = new SerializedSceneObject(selectedSO, true);
        }

        /// <summary>
        /// Clears a selected scene object. This must be called on every scene object <see cref="InitializeSO"/> has been called
        /// on, after operations on it are done.
        /// </summary>
        private void ClearSO()
        {
            if (selectedSO != null)
            {
                Animation animation = selectedSO.GetComponent<Animation>();
                if (animation != null)
                    animation.EditorStop();
            }

            if (soState != null)
            {
                soState.Restore();
                soState = null;
            }
        }

        #endregion

        #region Record/Playback

        /// <summary>
        /// Possible states the animation window can be in.
        /// </summary>
        private enum State
        {
            Empty,
            Normal,
            Recording,
            Playback
        }

        private State state = State.Empty;
        private SerializedSceneObject soState;
        private bool delayRecord = false;

        /// <summary>
        /// Transitions the window into a different state. Caller must validate state transitions.
        /// </summary>
        /// <param name="state">New state to transition to.</param>
        private void SwitchState(State state)
        {
            switch (this.state)
            {
                case State.Normal:
                {
                    switch (state)
                    {
                        case State.Playback:
                            StartPlayback();
                        break;
                        case State.Recording:
                            StartRecord();
                        break;
                        case State.Empty:
                            ClearSO();
                        break;
                    }
                }
                    break;
                case State.Playback:
                {
                    switch (state)
                    {
                        case State.Normal:
                            EndPlayback();
                            PreviewFrame(currentFrameIdx);
                        break;
                        case State.Recording:
                            EndPlayback();
                            StartRecord();
                        break;
                        case State.Empty:
                            EndPlayback();
                            ClearSO();
                        break;
                        }
                }
                    break;
                case State.Recording:
                {
                    switch (state)
                    {
                        case State.Normal:
                            EndRecord();
                            PreviewFrame(currentFrameIdx);
                        break;
                        case State.Playback:
                            EndRecord();
                            StartPlayback();
                        break;
                        case State.Empty:
                            EndRecord();
                            ClearSO();
                        break;
                    }
                }
                    break;
                case State.Empty:
                {
                    switch (state)
                    {
                        case State.Normal:
                            InitializeSO();
                            PreviewFrame(currentFrameIdx);
                            break;
                        case State.Playback:
                            InitializeSO();
                            StartPlayback();
                            break;
                        case State.Recording:
                            InitializeSO();
                            StartRecord();
                            break;
                    }
                }
                break;
            }

            this.state = state;
        }

        /// <summary>
        /// Plays back the animation on the currently selected object.
        /// </summary>
        private void StartPlayback()
        {
            EditorAnimClipTangents unused;
            clipInfo.Apply(out unused);

            Animation animation = selectedSO.GetComponent<Animation>();
            if (animation != null && clipInfo.clip != null)
            {
                float time = guiCurveEditor.GetTimeForFrame(currentFrameIdx);

                animation.EditorPlay(clipInfo.clip, time);
            }

            playButton.Value = true;
        }

        /// <summary>
        /// Ends playback started with <see cref="StartPlayback"/>
        /// </summary>
        private void EndPlayback()
        {
            PreviewFrame(currentFrameIdx);
            playButton.Value = false;
        }

        /// <summary>
        /// Updates the visible animation to display the provided frame.
        /// </summary>
        /// <param name="frameIdx">Index of the animation frame to display.</param>
        private void PreviewFrame(int frameIdx)
        {
            if (selectedSO == null)
                return;

            Animation animation = selectedSO.GetComponent<Animation>();
            if (animation != null && clipInfo.clip != null)
            {
                float time = guiCurveEditor.GetTimeForFrame(frameIdx);

                animation.EditorPlay(clipInfo.clip, time, true);
            }
        }

        /// <summary>
        /// Start recording modifications made to the selected scene object.
        /// </summary>
        private void StartRecord()
        {
            float time = guiCurveEditor.GetTimeForFrame(currentFrameIdx);
            if (RecordState(time))
            {
                ApplyClipChanges();
                guiCurveEditor.Redraw();
            }

            recordButton.Value = true;
        }

        /// <summary>
        /// Stops recording modifications made to the selected scene object.
        /// </summary>
        private void EndRecord()
        {
            recordButton.Value = false;
        }

        /// <summary>
        /// Iterates over all curve path fields and records their current state. If the state differs from the current
        /// curve values, new keyframes are added.
        /// </summary>
        /// <param name="time">Time for which to record the state, in seconds.</param>
        /// <returns>True if any changes were recorded, false otherwise.</returns>
        private bool RecordState(float time)
        {
            Action<EdAnimationCurve, float, float> addOrUpdateKeyframe = (curve, keyTime, keyValue) =>
            {
                KeyFrame[] keyframes = curve.KeyFrames;
                int keyframeIdx = -1;
                for (int i = 0; i < keyframes.Length; i++)
                {
                    if (MathEx.ApproxEquals(keyframes[i].time, time))
                    {
                        keyframeIdx = i;
                        break;
                    }
                }

                if (keyframeIdx != -1)
                    curve.UpdateKeyframe(keyframeIdx, keyTime, keyValue);
                else
                    curve.AddKeyframe(keyTime, keyValue);

                curve.Apply();
            };

            bool changesMade = false;
            foreach (var KVP in clipInfo.curves)
            {
                string suffix;
                SerializableProperty property = Animation.FindProperty(selectedSO, KVP.Key, out suffix);

                if (property == null)
                    continue;

                switch (KVP.Value.type)
                {
                    case SerializableProperty.FieldType.Vector2:
                        {
                            Vector2 value = property.GetValue<Vector2>();

                            for (int i = 0; i < 2; i++)
                            {
                                float curveVal = KVP.Value.curveInfos[i].curve.Evaluate(time);
                                if (!MathEx.ApproxEquals(value[i], curveVal, 0.001f))
                                {
                                    addOrUpdateKeyframe(KVP.Value.curveInfos[i].curve, time, value[i]);
                                    changesMade = true;
                                }
                            }
                        }
                        break;
                    case SerializableProperty.FieldType.Vector3:
                        {
                            Vector3 value = property.GetValue<Vector3>();

                            for (int i = 0; i < 3; i++)
                            {
                                float curveVal = KVP.Value.curveInfos[i].curve.Evaluate(time);
                                if (!MathEx.ApproxEquals(value[i], curveVal, 0.001f))
                                { 
                                    addOrUpdateKeyframe(KVP.Value.curveInfos[i].curve, time, value[i]);
                                    changesMade = true;
                                }
                            }
                        }
                        break;
                    case SerializableProperty.FieldType.Vector4:
                        {
                            if (property.InternalType == typeof(Vector4))
                            {
                                Vector4 value = property.GetValue<Vector4>();

                                for (int i = 0; i < 4; i++)
                                {
                                    float curveVal = KVP.Value.curveInfos[i].curve.Evaluate(time);
                                    if (!MathEx.ApproxEquals(value[i], curveVal, 0.001f))
                                    { 
                                        addOrUpdateKeyframe(KVP.Value.curveInfos[i].curve, time, value[i]);
                                        changesMade = true;
                                    }
                                }
                            }
                            else if (property.InternalType == typeof(Quaternion))
                            {
                                Quaternion value = property.GetValue<Quaternion>();

                                for (int i = 0; i < 4; i++)
                                {
                                    float curveVal = KVP.Value.curveInfos[i].curve.Evaluate(time);
                                    if (!MathEx.ApproxEquals(value[i], curveVal, 0.001f))
                                    { 
                                        addOrUpdateKeyframe(KVP.Value.curveInfos[i].curve, time, value[i]);
                                        changesMade = true;
                                    }
                                }
                            }
                        }
                        break;
                    case SerializableProperty.FieldType.Color:
                        {
                            Color value = property.GetValue<Color>();

                            for (int i = 0; i < 4; i++)
                            {
                                float curveVal = KVP.Value.curveInfos[i].curve.Evaluate(time);
                                if (!MathEx.ApproxEquals(value[i], curveVal, 0.001f))
                                { 
                                    addOrUpdateKeyframe(KVP.Value.curveInfos[i].curve, time, value[i]);
                                    changesMade = true;
                                }
                            }
                        }
                        break;
                    case SerializableProperty.FieldType.Bool:
                        {
                            bool value = property.GetValue<bool>();

                            bool curveVal = KVP.Value.curveInfos[0].curve.Evaluate(time) > 0.0f;
                            if (value != curveVal)
                            { 
                                addOrUpdateKeyframe(KVP.Value.curveInfos[0].curve, time, value ? 1.0f : -1.0f);
                                changesMade = true;
                            }
                        }
                        break;
                    case SerializableProperty.FieldType.Int:
                        {
                            int value = property.GetValue<int>();

                            int curveVal = (int)KVP.Value.curveInfos[0].curve.Evaluate(time);
                            if (value != curveVal)
                            { 
                                addOrUpdateKeyframe(KVP.Value.curveInfos[0].curve, time, value);
                                changesMade = true;
                            }
                        }
                        break;
                    case SerializableProperty.FieldType.Float:
                        {
                            float value = property.GetValue<float>();

                            float curveVal = KVP.Value.curveInfos[0].curve.Evaluate(time);
                            if (!MathEx.ApproxEquals(value, curveVal, 0.001f))
                            { 
                                addOrUpdateKeyframe(KVP.Value.curveInfos[0].curve, time, value);
                                changesMade = true;
                            }
                        }
                        break;
                }
            }

            return changesMade;
        }
        #endregion

        #region Curve display

        private int currentFrameIdx;
        private int fps = 1;

        /// <summary>
        /// Sampling rate of the animation in frames per second. Determines granularity at which positions keyframes can be
        /// placed.
        /// </summary>
        internal int FPS
        {
            get { return fps; }
            set { guiCurveEditor.SetFPS(value); fps = MathEx.Max(value, 1); }
        }

        /// <summary>
        /// Changes the currently selected frame in the curve display.
        /// </summary>
        /// <param name="frameIdx">Index of the frame to select.</param>
        private void SetCurrentFrame(int frameIdx)
        {
            currentFrameIdx = Math.Max(0, frameIdx);

            frameInputField.Value = currentFrameIdx;
            guiCurveEditor.SetMarkedFrame(currentFrameIdx);

            float time = guiCurveEditor.GetTimeForFrame(currentFrameIdx);

            List<GUIAnimFieldPathValue> values = new List<GUIAnimFieldPathValue>();
            foreach (var kvp in clipInfo.curves)
            {
                GUIAnimFieldPathValue fieldValue = new GUIAnimFieldPathValue();
                fieldValue.path = kvp.Key;

                switch (kvp.Value.type)
                {
                    case SerializableProperty.FieldType.Vector2:
                        {
                            Vector2 value = new Vector2();

                            for (int i = 0; i < 2; i++)
                                value[i] = kvp.Value.curveInfos[i].curve.Evaluate(time, false);

                            fieldValue.value = value;
                        }
                        break;
                    case SerializableProperty.FieldType.Vector3:
                        {
                            Vector3 value = new Vector3();

                            for (int i = 0; i < 3; i++)
                                value[i] = kvp.Value.curveInfos[i].curve.Evaluate(time, false);

                            fieldValue.value = value;
                        }
                        break;
                    case SerializableProperty.FieldType.Vector4:
                        {
                            Vector4 value = new Vector4();

                            for (int i = 0; i < 4; i++)
                                value[i] = kvp.Value.curveInfos[i].curve.Evaluate(time, false);

                            fieldValue.value = value;
                        }
                        break;
                    case SerializableProperty.FieldType.Color:
                        {
                            Color value = new Color();

                            for (int i = 0; i < 4; i++)
                                value[i] = kvp.Value.curveInfos[i].curve.Evaluate(time, false);

                            fieldValue.value = value;
                        }
                        break;
                    case SerializableProperty.FieldType.Bool:
                    case SerializableProperty.FieldType.Int:
                    case SerializableProperty.FieldType.Float:
                        fieldValue.value = kvp.Value.curveInfos[0].curve.Evaluate(time, false); ;
                        break;
                }

                values.Add(fieldValue);
            }

            guiFieldDisplay.SetDisplayValues(values.ToArray());
        }

        /// <summary>
        /// Returns a list of all animation curves that should be displayed in the curve display.
        /// </summary>
        /// <returns>Array of curves to display.</returns>
        private CurveDrawInfo[] GetDisplayedCurves()
        {
            List<CurveDrawInfo> curvesToDisplay = new List<CurveDrawInfo>();

            if (selectedFields.Count == 0) // Display all if nothing is selected
            {
                if (clipInfo == null)
                    return curvesToDisplay.ToArray();

                foreach (var curve in clipInfo.curves)
                {
                    for (int i = 0; i < curve.Value.curveInfos.Length; i++)
                        curvesToDisplay.Add(curve.Value.curveInfos[i]);
                }
            }
            else
            {
                for (int i = 0; i < selectedFields.Count; i++)
                {
                    CurveDrawInfo[] curveInfos;
                    if (TryGetCurve(selectedFields[i], out curveInfos))
                        curvesToDisplay.AddRange(curveInfos);
                }
            }

            return curvesToDisplay.ToArray();
        }

        /// <summary>
        /// Returns width/height required to show the entire contents of the currently displayed curves.
        /// </summary>
        /// <returns>Width/height of the curve area, in curve space (value, time).</returns>
        private Vector2 GetOptimalRange()
        {
            CurveDrawInfo[] curvesToDisplay = GetDisplayedCurves();

            float xRange;
            float yRange;
            CalculateRange(curvesToDisplay, out xRange, out yRange);

            // Add padding to y range
            yRange *= 1.05f;

            // Don't allow zero range
            if (xRange == 0.0f)
                xRange = 60.0f;

            if (yRange == 0.0f)
                yRange = 10.0f;

            return new Vector2(xRange, yRange);
        }

        /// <summary>
        /// Calculates an unique color for each animation curve.
        /// </summary>
        private void UpdateCurveColors()
        {
            int globalCurveIdx = 0;
            foreach (var curveGroup in clipInfo.curves)
            {
                for (int i = 0; i < curveGroup.Value.curveInfos.Length; i++)
                    curveGroup.Value.curveInfos[i].color = GUICurveDrawing.GetUniqueColor(globalCurveIdx++);
            }
        }

        /// <summary>
        /// Updates the curve display with currently selected curves.
        /// </summary>
        /// <param name="allowReduce">Normally the curve display will expand if newly selected curves cover a larger area
        ///                           than currently available, but the area won't be reduced if the selected curves cover
        ///                           a smaller area. Set this to true to allow the area to be reduced.</param>
        private void UpdateDisplayedCurves(bool allowReduce = false)
        {
            CurveDrawInfo[] curvesToDisplay = GetDisplayedCurves();
            guiCurveEditor.SetCurves(curvesToDisplay);

            Vector2 newRange = GetOptimalRange();
            if (!allowReduce)
            {
                // Don't reduce visible range
                newRange.x = Math.Max(newRange.x, guiCurveEditor.Range.x);
                newRange.y = Math.Max(newRange.y, guiCurveEditor.Range.y);
            }

            guiCurveEditor.Range = newRange;
            UpdateScrollBarSize();
        }

        #endregion 

        #region Field display
        private List<string> selectedFields = new List<string>();

        /// <summary>
        /// Registers a new animation curve field.
        /// </summary>
        /// <param name="path">Path of the field, see <see cref="GUIFieldSelector.OnElementSelected"/></param>
        /// <param name="type">Type of the field (float, vector, etc.)</param>
        private void AddNewField(string path, SerializableProperty.FieldType type)
        {
            bool noSelection = selectedFields.Count == 0;
            
            switch (type)
            {
                case SerializableProperty.FieldType.Vector4:
                    {
                        FieldAnimCurves fieldCurves = new FieldAnimCurves();
                        fieldCurves.type = type;
                        fieldCurves.curveInfos = new CurveDrawInfo[4];

                        string[] subPaths = { ".x", ".y", ".z", ".w" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curveInfos[i].curve = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        clipInfo.curves[path] = fieldCurves;
                    }
                    break;
                case SerializableProperty.FieldType.Vector3:
                    {
                        FieldAnimCurves fieldCurves = new FieldAnimCurves();
                        fieldCurves.type = type;
                        fieldCurves.curveInfos = new CurveDrawInfo[3];

                        string[] subPaths = { ".x", ".y", ".z" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curveInfos[i].curve = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        clipInfo.curves[path] = fieldCurves;
                    }
                    break;
                case SerializableProperty.FieldType.Vector2:
                    {
                        FieldAnimCurves fieldCurves = new FieldAnimCurves();
                        fieldCurves.type = type;
                        fieldCurves.curveInfos = new CurveDrawInfo[2];

                        string[] subPaths = { ".x", ".y" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curveInfos[i].curve = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        clipInfo.curves[path] = fieldCurves;
                    }
                    break;
                case SerializableProperty.FieldType.Color:
                    {
                        FieldAnimCurves fieldCurves = new FieldAnimCurves();
                        fieldCurves.type = type;
                        fieldCurves.curveInfos = new CurveDrawInfo[4];

                        string[] subPaths = { ".r", ".g", ".b", ".a" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curveInfos[i].curve = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        clipInfo.curves[path] = fieldCurves;
                    }
                    break;
                default: // Primitive type
                    {
                        FieldAnimCurves fieldCurves = new FieldAnimCurves();
                        fieldCurves.type = type;
                        fieldCurves.curveInfos = new CurveDrawInfo[1];

                        fieldCurves.curveInfos[0].curve = new EdAnimationCurve();
                        selectedFields.Add(path);

                        clipInfo.curves[path] = fieldCurves;
                    }
                    break;
            }

            UpdateCurveColors();
            UpdateDisplayedFields();

            EditorApplication.SetProjectDirty();
            UpdateDisplayedCurves(noSelection);
        }

        /// <summary>
        /// Selects a new animation curve field, making the curve display in the curve display GUI element.
        /// </summary>
        /// <param name="path">Path of the field to display.</param>
        /// <param name="additive">If true the field will be shown along with any already selected fields, or if false
        ///                        only the provided field will be shown.</param>
        private void SelectField(string path, bool additive)
        {
            if (!additive)
                selectedFields.Clear();

            bool noSelection = selectedFields.Count == 0;
            if (!string.IsNullOrEmpty(path))
            {
                selectedFields.RemoveAll(x => { return x == path || IsPathParent(x, path); });
                selectedFields.Add(path);
            }

            guiFieldDisplay.SetSelection(selectedFields.ToArray());

            UpdateDisplayedCurves(noSelection);
        }

        /// <summary>
        /// Deletes all currently selecting fields, removing them their curves permanently.
        /// </summary>
        private void RemoveSelectedFields()
        {
            for (int i = 0; i < selectedFields.Count; i++)
                clipInfo.curves.Remove(GetSubPathParent(selectedFields[i]));

            UpdateCurveColors();
            UpdateDisplayedFields();

            selectedFields.Clear();
            EditorApplication.SetProjectDirty();
            UpdateDisplayedCurves();
        }

        /// <summary>
        /// Updates the GUI element displaying the current animation curve fields.
        /// </summary>
        private void UpdateDisplayedFields()
        {
            List<AnimFieldInfo> existingFields = new List<AnimFieldInfo>();
            foreach (var KVP in clipInfo.curves)
                existingFields.Add(new AnimFieldInfo(KVP.Key, KVP.Value, !clipInfo.isImported));

            guiFieldDisplay.SetFields(existingFields.ToArray());
        }
        #endregion

        #region Helpers
        /// <summary>
        /// Returns the size of the curve editor GUI element.
        /// </summary>
        /// <returns>Width/height of the curve editor, in pixels.</returns>
        private Vector2I GetCurveEditorSize()
        {
            Vector2I output = new Vector2I();
            output.x = Math.Max(0, Width - FIELD_DISPLAY_WIDTH - scrollBarWidth);
            output.y = Math.Max(0, Height - buttonLayoutHeight - scrollBarHeight);

            return output;
        }

        /// <summary>
        /// Calculates the total range covered by a set of curves.
        /// </summary>
        /// <param name="curveInfos">Curves to calculate range for.</param>
        /// <param name="xRange">Maximum time value present in the curves.</param>
        /// <param name="yRange">Maximum absolute curve value present in the curves.</param>
        private static void CalculateRange(CurveDrawInfo[] curveInfos, out float xRange, out float yRange)
        {
            // Note: This only evaluates at keyframes, we should also evaluate in-between in order to account for steep
            // tangents
            xRange = 0.0f;
            yRange = 0.0f;

            foreach (var curveInfo in curveInfos)
            {
                KeyFrame[] keyframes = curveInfo.curve.KeyFrames;

                foreach (var key in keyframes)
                {
                    xRange = Math.Max(xRange, key.time);
                    yRange = Math.Max(yRange, Math.Abs(key.value));
                }
            }
        }

        /// <summary>
        /// Attempts to find a curve field at the specified path.
        /// </summary>
        /// <param name="path">Path of the curve field to look for.</param>
        /// <param name="curveInfos">One or multiple curves found for the specific path (one field can have multiple curves
        ///                          if it is a complex type, like a vector).</param>
        /// <returns>True if the curve field was found, false otherwise.</returns>
        private bool TryGetCurve(string path, out CurveDrawInfo[] curveInfos)
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

            FieldAnimCurves fieldCurves;
            if (clipInfo.curves.TryGetValue(parentPath, out fieldCurves))
            {
                if (!string.IsNullOrEmpty(subPathSuffix))
                {
                    if (subPathSuffix == ".x" || subPathSuffix == ".r")
                    {
                        curveInfos = new [] { fieldCurves.curveInfos[0] };
                        return true;
                    }
                    else if (subPathSuffix == ".y" || subPathSuffix == ".g")
                    {
                        curveInfos = new[] { fieldCurves.curveInfos[1] };
                        return true;
                    }
                    else if (subPathSuffix == ".z" || subPathSuffix == ".b")
                    {
                        curveInfos = new[] { fieldCurves.curveInfos[2] };
                        return true;
                    }
                    else if (subPathSuffix == ".w" || subPathSuffix == ".a")
                    {
                        curveInfos = new[] { fieldCurves.curveInfos[3] };
                        return true;
                    }
                }
                else
                {
                    curveInfos = fieldCurves.curveInfos;
                    return true;
                }
            }

            curveInfos = new CurveDrawInfo[0];
            return false;
        }

        /// <summary>
        /// Checks if one curve field path a parent of the other.
        /// </summary>
        /// <param name="child">Path to check if it is a child of <paramref name="parent"/>.</param>
        /// <param name="parent">Path to check if it is a parent of <paramref name="child"/>.</param>
        /// <returns>True if <paramref name="child"/> is a child of <paramref name="parent"/>.</returns>
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

        /// <summary>
        /// If a path has sub-elements (e.g. .x, .r), returns a path without those elements. Otherwise returns the original
        /// path.
        /// </summary>
        /// <param name="path">Path to check.</param>
        /// <returns>Path without sub-elements.</returns>
        private string GetSubPathParent(string path)
        {
            int index = path.LastIndexOf(".");
            if (index == -1)
                return path;

            return path.Substring(0, index);
        }

        #endregion

        #region Input callbacks
        /// <summary>
        /// Triggered when the user presses a mouse button.
        /// </summary>
        /// <param name="ev">Information about the mouse press event.</param>
        private void OnPointerPressed(PointerEvent ev)
        {
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

        /// <summary>
        /// Triggered when the user moves the mouse.
        /// </summary>
        /// <param name="ev">Information about the mouse move event.</param>
        private void OnPointerMoved(PointerEvent ev)
        {
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

        /// <summary>
        /// Triggered when the user releases a mouse button.
        /// </summary>
        /// <param name="ev">Information about the mouse release event.</param>
        private void OnPointerReleased(PointerEvent ev)
        {
            if (isDragInProgress)
            {
                Cursor.Show();
                Cursor.ClipDisable();
            }

            isButtonHeld = false;
            isDragInProgress = false;

            guiCurveEditor.OnPointerReleased(ev);
        }

        /// <summary>
        /// Triggered when the user releases a keyboard button.
        /// </summary>
        /// <param name="ev">Information about the keyboard release event.</param>
        private void OnButtonUp(ButtonEvent ev)
        {
            guiCurveEditor.OnButtonUp(ev);
        }
        #endregion

        #region General callbacks
        /// <summary>
        /// Triggered by the field selector, when user selects a new curve field.
        /// </summary>
        /// <param name="path">Path of the selected curve field.</param>
        /// <param name="type">Type of the selected curve field (float, vector, etc.).</param>
        private void OnFieldAdded(string path, SerializableProperty.FieldType type)
        {
            // Remove the root scene object from the path (we know which SO it is, no need to hardcode its name in the path)
            string pathNoRoot = path.TrimStart('/');
            int separatorIdx = pathNoRoot.IndexOf("/");
            if (separatorIdx == -1 || (separatorIdx + 1) >= pathNoRoot.Length)
                return;

            pathNoRoot = pathNoRoot.Substring(separatorIdx + 1, pathNoRoot.Length - separatorIdx - 1);

            AddNewField(pathNoRoot, type);
            ApplyClipChanges();
        }

        /// <summary>
        /// Triggered when the user moves or resizes the horizontal scrollbar.
        /// </summary>
        /// <param name="position">New position of the scrollbar, in range [0, 1].</param>
        /// <param name="size">New size of the scrollbar, in range [0, 1].</param>
        private void OnHorzScrollOrResize(float position, float size)
        {
            SetHorzScrollbarProperties(position, size);
        }

        /// <summary>
        /// Triggered when the user moves or resizes the vertical scrollbar.
        /// </summary>
        /// <param name="position">New position of the scrollbar, in range [0, 1].</param>
        /// <param name="size">New size of the scrollbar, in range [0, 1].</param>
        private void OnVertScrollOrResize(float position, float size)
        {
            SetVertScrollbarProperties(position, size);
        }

        /// <summary>
        /// Triggered when the user selects a new curve field.
        /// </summary>
        /// <param name="path">Path of the selected curve field.</param>
        private void OnFieldSelected(string path)
        {
            bool additive = Input.IsButtonHeld(ButtonCode.LeftShift) || Input.IsButtonHeld(ButtonCode.RightShift);
            SelectField(path, additive);
        }

        /// <summary>
        /// Triggered when the user selects a new scene object or a resource.
        /// </summary>
        /// <param name="sceneObjects">Newly selected scene objects.</param>
        /// <param name="resourcePaths">Newly selected resources.</param>
        private void OnSelectionChanged(SceneObject[] sceneObjects, string[] resourcePaths)
        {
            // While recording allow other objects to be selected so the user can modify them
            if (state == State.Recording)
                return;

            UpdateSelectedSO(false);
        }

        /// <summary>
        /// Triggered when the user selects a new frame in the curve display.
        /// </summary>
        /// <param name="frameIdx">Index of the selected frame.</param>
        private void OnFrameSelected(int frameIdx)
        {
            SetCurrentFrame(frameIdx);
            PreviewFrame(currentFrameIdx);

            // HACK: Skip checking for record changes this frame, to give the preview a chance to update, otherwise
            // the changes would be detected any time a frame is delayed. A proper fix for this would be to force the
            // animation to be evaluated synchronously when PreviewFrame is called.
            delayRecord = true;
        }

        /// <summary>
        /// Triggered when the user changed (add, removed or modified) animation events in the curve display.
        /// </summary>
        private void OnEventsChanged()
        {
            clipInfo.events = guiCurveEditor.Events;
            EditorApplication.SetProjectDirty();
        }
        #endregion
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

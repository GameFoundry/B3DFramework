//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.IO;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Scene-Editor
     *  @{
     */

    /// <summary>
    /// Displays the scene view camera and various scene controls.
    /// </summary>
    internal sealed class SceneWindow : EditorWindow, IGlobalShortcuts
    {
        internal const string ToggleProfilerOverlayBinding = "ToggleProfilerOverlay";
        internal const string ViewToolBinding = "ViewTool";
        internal const string MoveToolBinding = "MoveTool";
        internal const string RotateToolBinding = "RotateTool";
        internal const string ScaleToolBinding = "ScaleTool";
        internal const string FrameBinding = "SceneFrame";

        private const int HeaderHeight = 20;
        private const float DefaultPlacementDepth = 5.0f;
        private static readonly Color ClearColor = new Color(0.0f, 0.3685f, 0.7969f);
        private const string ProfilerOverlayActiveKey = "_Internal_ProfilerOverlayActive";
        private const int HandleAxesGUISize = 50;
        private const int HandleAxesGUIPaddingX = 10;
        private const int HandleAxesGUIPaddingY = 5;

        private Camera camera;
        private SceneCamera cameraController;
        private RenderTexture renderTexture;
        private GUILayoutY mainLayout;
        private GUIPanel rtPanel;
        private GUIButton focusCatcher;

        private GUIRenderTexture renderTextureGUI;
        private SceneGrid sceneGrid;
        private SceneSelection sceneSelection;
        private SceneGizmos sceneGizmos;
        private SceneHandles sceneHandles;

        private GUIToggle viewButton;
        private GUIToggle moveButton;
        private GUIToggle rotateButton;
        private GUIToggle scaleButton;

        private GUIToggle localCoordButton;
        private GUIToggle worldCoordButton;

        private GUIToggle pivotButton;
        private GUIToggle centerButton;

        private GUIToggle moveSnapButton;
        private GUIFloatField moveSnapInput;

        private GUIToggle rotateSnapButton;
        private GUIFloatField rotateSnapInput;

        private SceneAxesGUI sceneAxesGUI;

        private bool hasContentFocus = false;
        private bool HasContentFocus { get { return HasFocus && hasContentFocus; } }

        private int editorSettingsHash = int.MaxValue;

        private VirtualButton frameKey;

        // Tool shortcuts
        private VirtualButton viewToolKey;
        private VirtualButton moveToolKey;
        private VirtualButton rotateToolKey;
        private VirtualButton scaleToolKey;

        // Profiler overlay
        private ProfilerOverlay activeProfilerOverlay;
        private Camera profilerCamera;
        private VirtualButton toggleProfilerOverlayKey;

        // Drag & drop
        private bool dragActive;
        private SceneObject draggedSO;
        private Vector3 draggedSOOffset;
        private GUITexture dragSelection;
        private bool isDraggingSelection;
        private Vector2I dragSelectionStart;
        private Vector2I dragSelectionEnd;
        private Vector2I mouseDownPosition;
        private GUIPanel selectionPanel;

        /// <summary>
        /// Returns the scene camera.
        /// </summary>
        public Camera Camera
        {
            get { return camera; }
        }

        /// <summary>
        /// Determines scene camera's projection type.
        /// </summary>
        internal ProjectionType ProjectionType
        {
            get { return cameraController.ProjectionType; }
            set { cameraController.ProjectionType = value; sceneAxesGUI.ProjectionType = value; }
        }

        /// <summary>
        /// Constructs a new scene window.
        /// </summary>
        internal SceneWindow()
        { }

        /// <summary>
        /// Opens a scene window if its not open already.
        /// </summary>
        [MenuItem("Windows/Scene", ButtonModifier.CtrlAlt, ButtonCode.S, 6000)]
        private static void OpenSceneWindow()
        {
            OpenWindow<SceneWindow>();
        }

        /// <summary>
        /// Focuses on the currently selected object.
        /// </summary>
        [MenuItem("Tools/Frame Selected", ButtonModifier.None, ButtonCode.F, 9275, true)]
        private static void OpenSettingsWindow()
        {
            SceneWindow window = GetWindow<SceneWindow>();
            if (window != null)
                window.cameraController.FrameSelected();
        }

        /// <summary>
        /// Switches the active tool to the view tool.
        /// </summary>
        [MenuItem("Tools/View", ButtonModifier.Ctrl, ButtonCode.Q, 9274, true)]
        private static void SetViewTool()
        {
            SceneWindow window = GetWindow<SceneWindow>();
            if (window != null)
                window.OnSceneToolButtonClicked(SceneViewTool.View);
        }

        /// <summary>
        /// Switches the active tool to the move tool.
        /// </summary>
        [MenuItem("Tools/Move", ButtonModifier.Ctrl, ButtonCode.W, 9273)]
        private static void SetMoveTool()
        {
            SceneWindow window = GetWindow<SceneWindow>();
            if (window != null)
                window.OnSceneToolButtonClicked(SceneViewTool.Move);
        }

        /// <summary>
        /// Switches the active tool to the rotate tool.
        /// </summary>
        [MenuItem("Tools/Rotate", ButtonModifier.Ctrl, ButtonCode.E, 9272)]
        private static void SetRotateTool()
        {
            SceneWindow window = GetWindow<SceneWindow>();
            if (window != null)
                window.OnSceneToolButtonClicked(SceneViewTool.Rotate);
        }

        /// <summary>
        /// Switches the active tool to the scale tool.
        /// </summary>
        [MenuItem("Tools/Scale", ButtonModifier.Ctrl, ButtonCode.R, 9271)]
        private static void SetScaleTool()
        {
            SceneWindow window = GetWindow<SceneWindow>();
            if (window != null)
                window.OnSceneToolButtonClicked(SceneViewTool.Scale);
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Scene");
        }

        private void OnInitialize()
        {
            mainLayout = GUI.AddLayoutY();

            GUIContent viewIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.View), 
                new LocEdString("View"));
            GUIContent moveIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.Move), 
                new LocEdString("Move"));
            GUIContent rotateIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.Rotate), 
                new LocEdString("Rotate"));
            GUIContent scaleIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.Scale), 
                new LocEdString("Scale"));

            GUIContent localIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.Local), 
                new LocEdString("Local"));
            GUIContent worldIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.World), 
                new LocEdString("World"));

            GUIContent pivotIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.Pivot), 
                new LocEdString("Pivot"));
            GUIContent centerIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.Center), 
                new LocEdString("Center"));

            GUIContent moveSnapIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.MoveSnap), 
                new LocEdString("Move snap"));
            GUIContent rotateSnapIcon = new GUIContent(EditorBuiltin.GetSceneWindowIcon(SceneWindowIcon.RotateSnap), 
                new LocEdString("Rotate snap"));

            GUIToggleGroup handlesTG = new GUIToggleGroup();
            viewButton = new GUIToggle(viewIcon, handlesTG, EditorStyles.Button, GUIOption.FlexibleWidth(35));
            moveButton = new GUIToggle(moveIcon, handlesTG, EditorStyles.Button, GUIOption.FlexibleWidth(35));
            rotateButton = new GUIToggle(rotateIcon, handlesTG, EditorStyles.Button, GUIOption.FlexibleWidth(35));
            scaleButton = new GUIToggle(scaleIcon, handlesTG, EditorStyles.Button, GUIOption.FlexibleWidth(35));

            GUIToggleGroup coordModeTG = new GUIToggleGroup();
            localCoordButton = new GUIToggle(localIcon, coordModeTG, EditorStyles.Button, GUIOption.FlexibleWidth(75));
            worldCoordButton = new GUIToggle(worldIcon, coordModeTG, EditorStyles.Button, GUIOption.FlexibleWidth(75));

            GUIToggleGroup pivotModeTG = new GUIToggleGroup();
            pivotButton = new GUIToggle(pivotIcon, pivotModeTG, EditorStyles.Button, GUIOption.FlexibleWidth(35));
            centerButton = new GUIToggle(centerIcon, pivotModeTG, EditorStyles.Button, GUIOption.FlexibleWidth(35));

            moveSnapButton = new GUIToggle(moveSnapIcon, EditorStyles.Button, GUIOption.FlexibleWidth(35));
            moveSnapInput = new GUIFloatField("", GUIOption.FlexibleWidth(35));

            rotateSnapButton = new GUIToggle(rotateSnapIcon, EditorStyles.Button, GUIOption.FlexibleWidth(35));
            rotateSnapInput = new GUIFloatField("", GUIOption.FlexibleWidth(35));

            viewButton.OnClick += () => OnSceneToolButtonClicked(SceneViewTool.View);
            moveButton.OnClick += () => OnSceneToolButtonClicked(SceneViewTool.Move);
            rotateButton.OnClick += () => OnSceneToolButtonClicked(SceneViewTool.Rotate);
            scaleButton.OnClick += () => OnSceneToolButtonClicked(SceneViewTool.Scale);

            localCoordButton.OnClick += () => OnCoordinateModeButtonClicked(HandleCoordinateMode.Local);
            worldCoordButton.OnClick += () => OnCoordinateModeButtonClicked(HandleCoordinateMode.World);

            pivotButton.OnClick += () => OnPivotModeButtonClicked(HandlePivotMode.Pivot);
            centerButton.OnClick += () => OnPivotModeButtonClicked(HandlePivotMode.Center);

            moveSnapButton.OnToggled += (bool active) => OnMoveSnapToggled(active);
            moveSnapInput.OnChanged += (float value) => OnMoveSnapValueChanged(value);

            rotateSnapButton.OnToggled += (bool active) => OnRotateSnapToggled(active);
            rotateSnapInput.OnChanged += (float value) => OnRotateSnapValueChanged(value);

            GUILayout handlesLayout = mainLayout.AddLayoutX();
            handlesLayout.AddElement(viewButton);
            handlesLayout.AddElement(moveButton);
            handlesLayout.AddElement(rotateButton);
            handlesLayout.AddElement(scaleButton);
            handlesLayout.AddSpace(10);
            handlesLayout.AddElement(localCoordButton);
            handlesLayout.AddElement(worldCoordButton);
            handlesLayout.AddSpace(10);
            handlesLayout.AddElement(pivotButton);
            handlesLayout.AddElement(centerButton);
            handlesLayout.AddFlexibleSpace();
            handlesLayout.AddElement(moveSnapButton);
            handlesLayout.AddElement(moveSnapInput);
            handlesLayout.AddSpace(10);
            handlesLayout.AddElement(rotateSnapButton);
            handlesLayout.AddElement(rotateSnapInput);

            GUIPanel mainPanel = mainLayout.AddPanel();
            rtPanel = mainPanel.AddPanel();

            selectionPanel = mainPanel.AddPanel(-1);

            GUIPanel sceneAxesPanel = mainPanel.AddPanel(-1);
            sceneAxesGUI = new SceneAxesGUI(this, sceneAxesPanel, HandleAxesGUISize, HandleAxesGUISize, ProjectionType.Perspective);

            focusCatcher = new GUIButton("", EditorStyles.Blank);
            focusCatcher.OnFocusGained += () => hasContentFocus = true;
            focusCatcher.OnFocusLost += () => hasContentFocus = false;

            GUIPanel focusPanel = GUI.AddPanel(-2);
            focusPanel.AddElement(focusCatcher);

            toggleProfilerOverlayKey = new VirtualButton(ToggleProfilerOverlayBinding);
            viewToolKey = new VirtualButton(ViewToolBinding);
            moveToolKey = new VirtualButton(MoveToolBinding);
            rotateToolKey = new VirtualButton(RotateToolBinding);
            scaleToolKey = new VirtualButton(ScaleToolBinding);
            frameKey = new VirtualButton(FrameBinding);

            UpdateRenderTexture(Width, Height - HeaderHeight);
            UpdateProfilerOverlay();
        }

        private void OnDestroy()
        {
            if (camera != null)
            {
                camera.SceneObject.Destroy();
                camera = null;
            }

            sceneAxesGUI.Destroy();
            sceneAxesGUI = null;
        }

        /// <summary>
        /// Deletes all currently selected objects.
        /// </summary>
        private void DeleteSelection()
        {
            SceneObject[] selectedObjects = Selection.SceneObjects;
            CleanDuplicates(ref selectedObjects);

            if (selectedObjects.Length > 0)
            {
                foreach (var so in selectedObjects)
                {
                    string message = "Deleted " + so.Name;
                    UndoRedo.DeleteSO(so, message);
                }

                EditorApplication.SetSceneDirty();
            }
        }

        /// <summary>
        /// Duplicates all currently selected objects.
        /// </summary>
        private void DuplicateSelection()
        {
            SceneObject[] selectedObjects = Selection.SceneObjects;
            CleanDuplicates(ref selectedObjects);

            if (selectedObjects.Length > 0)
            {
                string message;
                if (selectedObjects.Length == 1)
                    message = "Duplicated " + selectedObjects[0].Name;
                else
                    message = "Duplicated " + selectedObjects.Length + " elements";

                UndoRedo.CloneSO(selectedObjects, message);
                EditorApplication.SetSceneDirty();
            }
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnRenamePressed()
        {
            // Do nothing
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnDuplicatePressed()
        {
            DuplicateSelection();
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnDeletePressed()
        {
            DeleteSelection();
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnCopyPressed()
        {
            // Do nothing
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnCutPressed()
        {
            // Do nothing
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnPastePressed()
        {
            // Do nothing
        }

        /// <summary>
        /// Orients the camera so it looks along the provided axis.
        /// </summary>
        /// <param name="axis">Axis to look along.</param>
        internal void LookAlong(Vector3 axis)
        {
            axis.Normalize();

            cameraController.LookAlong(axis);
            UpdateGridMode();
        }

        private void UpdateGridMode()
        {
            Vector3 forward = camera.SceneObject.Forward;

            if (camera.ProjectionType == ProjectionType.Perspective)
                sceneGrid.SetMode(GridMode.Perspective);
            else
            {
                float dotX = Vector3.Dot(forward, Vector3.XAxis);
                if (dotX >= 0.95f)
                    sceneGrid.SetMode(GridMode.OrthoX);
                else if (dotX <= -0.95f)
                    sceneGrid.SetMode(GridMode.OrthoNegX);
                else
                {
                    float dotY = Vector3.Dot(forward, Vector3.YAxis);
                    if (dotY >= 0.95f)
                        sceneGrid.SetMode(GridMode.OrthoY);
                    else if (dotY <= -0.95f)
                        sceneGrid.SetMode(GridMode.OrthoNegY);
                    else
                    {
                        float dotZ = Vector3.Dot(forward, Vector3.ZAxis);
                        if (dotZ >= 0.95f)
                            sceneGrid.SetMode(GridMode.OrthoZ);
                        else if (dotZ <= -0.95f)
                            sceneGrid.SetMode(GridMode.OrthoNegZ);
                        else
                            sceneGrid.SetMode(GridMode.Perspective);
                    }
                }
            }
        }
        
        /// <summary>
        /// Converts screen coordinates into coordinates relative to the scene view render texture.
        /// </summary>
        /// <param name="screenPos">Coordinates relative to the screen.</param>
        /// <param name="scenePos">Output coordinates relative to the scene view texture.</param>
        /// <returns>True if the coordinates are within the scene view texture, false otherwise.</returns>
        private bool ScreenToScenePos(Vector2I screenPos, out Vector2I scenePos)
        {
            scenePos = screenPos;
            Vector2I windowPos = ScreenToWindowPos(screenPos);

            Rect2I bounds = GUIUtility.CalculateBounds(renderTextureGUI, GUI);
            if (bounds.Contains(windowPos))
            {
                scenePos.x = windowPos.x - bounds.x;
                scenePos.y = windowPos.y - bounds.y;

                return true;
            }

            return false;
        }

        private void OnEditorUpdate()
        {
            if (HasFocus)
            {
                if (!Input.IsPointerButtonHeld(PointerButton.Right))
                {
                    if (VirtualInput.IsButtonDown(EditorApplication.DuplicateKey))
                        DuplicateSelection();
                    else if (VirtualInput.IsButtonDown(EditorApplication.DeleteKey))
                        DeleteSelection();
                    else if (VirtualInput.IsButtonDown(toggleProfilerOverlayKey))
                        EditorSettings.SetBool(ProfilerOverlayActiveKey, !EditorSettings.GetBool(ProfilerOverlayActiveKey));
                    else if(VirtualInput.IsButtonDown(viewToolKey))
                        EditorApplication.ActiveSceneTool = SceneViewTool.View;
                    else if(VirtualInput.IsButtonDown(moveToolKey))
                        EditorApplication.ActiveSceneTool = SceneViewTool.Move;
                    else if(VirtualInput.IsButtonDown(rotateToolKey))
                        EditorApplication.ActiveSceneTool = SceneViewTool.Rotate;
                    else if(VirtualInput.IsButtonDown(scaleToolKey))
                        EditorApplication.ActiveSceneTool = SceneViewTool.Scale;
                }
            }

            // Refresh GUI buttons if needed (in case someones changes the values from script)
            if (editorSettingsHash != EditorSettings.Hash)
            {
                UpdateButtonStates();
                UpdateProfilerOverlay();
                editorSettingsHash = EditorSettings.Hash;
            }

            // Update scene view handles and selection
            sceneGizmos.Draw();
            sceneGrid.Draw();

            bool handleActive = sceneHandles.IsActive() || sceneAxesGUI.IsActive();

            Vector2I scenePos;
            bool inBounds = ScreenToScenePos(Input.PointerPosition, out scenePos);
            bool dragResult = false;
            if (Input.IsPointerButtonUp(PointerButton.Left))
            {
                dragResult = EndDragSelection();
                if (sceneHandles.IsActive())
                    sceneHandles.ClearSelection();

                if (sceneAxesGUI.IsActive())
                    sceneAxesGUI.ClearSelection();
            } 
            else if (Input.IsPointerButtonDown(PointerButton.Left))
            {
                mouseDownPosition = scenePos;
            }

            bool draggedOver = DragDrop.DragInProgress || DragDrop.DropInProgress;
            draggedOver &= IsPointerHovering && inBounds && DragDrop.Type == DragDropType.Resource;

            if (draggedOver)
            {
                if (DragDrop.DropInProgress)
                {
                    dragActive = false;
                    if (draggedSO != null)
                    {
                        Selection.SceneObject = draggedSO;
                        EditorApplication.SetSceneDirty();
                    }
                    
                    draggedSO = null;
                }
                else
                {
                    if (!dragActive)
                    {
                        dragActive = true;

                        ResourceDragDropData dragData = (ResourceDragDropData)DragDrop.Data;

                        string[] draggedPaths = dragData.Paths;

                        for (int i = 0; i < draggedPaths.Length; i++)
                        {
                            ResourceMeta meta = ProjectLibrary.GetMeta(draggedPaths[i]);
                            if (meta != null)
                            {
                                if (meta.ResType == ResourceType.Mesh)
                                {
                                    if (!string.IsNullOrEmpty(draggedPaths[i]))
                                    {
                                        string meshName = Path.GetFileNameWithoutExtension(draggedPaths[i]);
                                        draggedSO = UndoRedo.CreateSO(meshName, "Created a new Renderable \"" + meshName + "\"");
                                        Mesh mesh = ProjectLibrary.Load<Mesh>(draggedPaths[i]);

                                        Renderable renderable = draggedSO.AddComponent<Renderable>();
                                        renderable.Mesh = mesh;
                                        if (mesh != null)
                                            draggedSOOffset = mesh.Bounds.Box.Center;
                                        else
                                            draggedSOOffset = Vector3.Zero;
                                    }

                                    break;
                                }
                                else if (meta.ResType == ResourceType.Prefab)
                                {
                                    if (!string.IsNullOrEmpty(draggedPaths[i]))
                                    {
                                        Prefab prefab = ProjectLibrary.Load<Prefab>(draggedPaths[i]);
                                        draggedSO = UndoRedo.Instantiate(prefab, "Instantiating " + prefab.Name);

                                        if (draggedSO != null)
                                        {
                                            AABox draggedObjBounds = EditorUtility.CalculateBounds(draggedSO);
                                            draggedSOOffset = draggedObjBounds.Center;
                                        }
                                        else
                                            draggedSOOffset = Vector3.Zero;
                                    }

                                    break;
                                }
                            }
                        }
                    }

                    if (draggedSO != null)
                    {
                        if (Input.IsButtonHeld(ButtonCode.Space))
                        {
                            SnapData snapData;
                            sceneSelection.Snap(scenePos, out snapData, new SceneObject[] {draggedSO});

                            Quaternion q = Quaternion.FromToRotation(Vector3.YAxis, snapData.normal);
                            draggedSO.Position = snapData.position;
                            draggedSO.Rotation = q;
                        }
                        else
                        {
                            Ray worldRay = camera.ScreenPointToRay(scenePos);
                            draggedSO.Position = worldRay * DefaultPlacementDepth - draggedSOOffset;
                        }
                    }
                }

                return;
            }
            else
            {
                if (dragActive)
                {
                    dragActive = false;

                    if (draggedSO != null)
                    {
                        draggedSO.Destroy();
                        draggedSO = null;
                    }
                }
            }

            if (HasContentFocus || IsPointerHovering)
            {
                cameraController.EnableInput(true);

                if (inBounds && HasContentFocus)
                {
                    if (Input.IsPointerButtonDown(PointerButton.Left))
                    {
                        Rect2I sceneAxesGUIBounds = new Rect2I(Width - HandleAxesGUISize - HandleAxesGUIPaddingX, 
                            HandleAxesGUIPaddingY, HandleAxesGUISize, HandleAxesGUISize);

                        if (sceneAxesGUIBounds.Contains(scenePos))
                            sceneAxesGUI.TrySelect(scenePos);
                        else
                            sceneHandles.TrySelect(scenePos);
                    }
                    else if (Input.IsPointerButtonHeld(PointerButton.Left) && !handleActive && !dragActive &&
                             draggedSO == null && scenePos != mouseDownPosition)
                    {
                        if (isDraggingSelection)
                            UpdateDragSelection(scenePos);
                        else
                            StartDragSelection(scenePos);
                    }
                    else if (Input.IsPointerButtonUp(PointerButton.Left))
                    {
                        if (!handleActive && !dragActive && !dragResult)
                        {
                            bool ctrlHeld = Input.IsButtonHeld(ButtonCode.LeftControl) ||
                                            Input.IsButtonHeld(ButtonCode.RightControl);

                            sceneSelection.PickObject(scenePos, ctrlHeld, new SceneObject[] {draggedSO});
                        }
                    }
                }
            }
            else
                cameraController.EnableInput(false);
            
            SceneHandles.BeginInput();
            sceneHandles.UpdateInput(scenePos, Input.PointerDelta);
            sceneHandles.Draw();

            sceneAxesGUI.UpdateInput(scenePos);
            sceneAxesGUI.Draw();
            SceneHandles.EndInput();

            sceneSelection.Draw();

            UpdateGridMode();

            if (VirtualInput.IsButtonDown(frameKey))
                cameraController.FrameSelected();
        }

        /// <inheritdoc/>
        protected override void WindowResized(int width, int height)
        {
            UpdateRenderTexture(width, height - HeaderHeight);

            base.WindowResized(width, height);
        }

        /// <inheritdoc/>
        protected override void FocusChanged(bool inFocus)
        {
            if (!inFocus)
            {
                sceneHandles.ClearSelection();
            }
        }

        /// <summary>
        /// Triggered when one of the scene tool buttons is clicked, changing the active scene handle.
        /// </summary>
        /// <param name="tool">Clicked scene tool to activate.</param>
        private void OnSceneToolButtonClicked(SceneViewTool tool)
        {
            EditorApplication.ActiveSceneTool = tool;
            editorSettingsHash = EditorSettings.Hash;
        }

        /// <summary>
        /// Triggered when one of the coordinate mode buttons is clicked, changing the active coordinate mode.
        /// </summary>
        /// <param name="mode">Clicked coordinate mode to activate.</param>
        private void OnCoordinateModeButtonClicked(HandleCoordinateMode mode)
        {
            EditorApplication.ActiveCoordinateMode = mode;
            editorSettingsHash = EditorSettings.Hash;
        }

        /// <summary>
        /// Triggered when one of the pivot buttons is clicked, changing the active pivot mode.
        /// </summary>
        /// <param name="mode">Clicked pivot mode to activate.</param>
        private void OnPivotModeButtonClicked(HandlePivotMode mode)
        {
            EditorApplication.ActivePivotMode = mode;
            editorSettingsHash = EditorSettings.Hash;
        }

        /// <summary>
        /// Triggered when the move snap button is toggled.
        /// </summary>
        /// <param name="active">Determins should be move snap be activated or deactivated.</param>
        private void OnMoveSnapToggled(bool active)
        {
            Handles.MoveHandleSnapActive = active;
            editorSettingsHash = EditorSettings.Hash;
        }

        /// <summary>
        /// Triggered when the move snap increment value changes.
        /// </summary>
        /// <param name="value">Value that determines in what increments to perform move snapping.</param>
        private void OnMoveSnapValueChanged(float value)
        {
            Handles.MoveSnapAmount = MathEx.Clamp(value, 0.01f, 1000.0f);
            editorSettingsHash = EditorSettings.Hash;
        }

        /// <summary>
        /// Triggered when the rotate snap button is toggled.
        /// </summary>
        /// <param name="active">Determins should be rotate snap be activated or deactivated.</param>
        private void OnRotateSnapToggled(bool active)
        {
            Handles.RotateHandleSnapActive = active;
            editorSettingsHash = EditorSettings.Hash;
        }

        /// <summary>
        /// Triggered when the rotate snap increment value changes.
        /// </summary>
        /// <param name="value">Value that determines in what increments to perform rotate snapping.</param>
        private void OnRotateSnapValueChanged(float value)
        {
            Handles.RotateSnapAmount = (Degree)MathEx.Clamp(value, 0.01f, 360.0f);
            editorSettingsHash = EditorSettings.Hash;
        }

        /// <summary>
        /// Updates toggle button states according to current editor options. This is useful if tools, coordinate mode,
        /// pivot or other scene view options have been modified externally.
        /// </summary>
        private void UpdateButtonStates()
        {
            switch (EditorApplication.ActiveSceneTool)
            {
                case SceneViewTool.View:
                    viewButton.Value = true;
                    break;
                case SceneViewTool.Move:
                    moveButton.Value = true;
                    break;
                case SceneViewTool.Rotate:
                    rotateButton.Value = true;
                    break;
                case SceneViewTool.Scale:
                    scaleButton.Value = true;
                    break;
            }

            switch (EditorApplication.ActiveCoordinateMode)
            {
                case HandleCoordinateMode.Local:
                    localCoordButton.Value = true;
                    break;
                case HandleCoordinateMode.World:
                    worldCoordButton.Value = true;
                    break;
            }

            switch (EditorApplication.ActivePivotMode)
            {
                case HandlePivotMode.Center:
                    centerButton.Value = true;
                    break;
                case HandlePivotMode.Pivot:
                    pivotButton.Value = true;
                    break;
            }

            if (Handles.MoveHandleSnapActive)
                moveSnapButton.Value = true;
            else
                moveSnapButton.Value = false;

            moveSnapInput.Value = Handles.MoveSnapAmount;

            if (Handles.RotateHandleSnapActive)
                rotateSnapButton.Value = true;
            else
                rotateSnapButton.Value = false;

            moveSnapInput.Value = Handles.RotateSnapAmount.Degrees;
        }

        /// <summary>
        /// Activates or deactivates the profiler overlay according to current editor settings.
        /// </summary>
        private void UpdateProfilerOverlay()
        {
            if (EditorSettings.GetBool(ProfilerOverlayActiveKey))
            {
                if (activeProfilerOverlay == null)
                {
                    SceneObject profilerSO = new SceneObject("EditorProfilerOverlay");
                    profilerCamera = profilerSO.AddComponent<Camera>();
                    profilerCamera.Viewport.Target = renderTexture;
                    profilerCamera.Viewport.ClearFlags = ClearFlags.Empty;
                    profilerCamera.Priority = 1;
                    profilerCamera.Layers = 0;
                    profilerCamera.RenderSettings.EnableHDR = false;

                    activeProfilerOverlay = profilerSO.AddComponent<ProfilerOverlay>();
                }
            }
            else
            {
                if (activeProfilerOverlay != null)
                {
                    activeProfilerOverlay.SceneObject.Destroy();
                    activeProfilerOverlay = null;
                    profilerCamera = null;
                }
            }
        }

        /// <summary>
        /// Creates the scene camera and updates the render texture. Should be called at least once before using the
        /// scene view. Should be called whenever the window is resized.
        /// </summary>
        /// <param name="width">Width of the scene render target, in pixels.</param>
        /// <param name="height">Height of the scene render target, in pixels.</param>
        private void UpdateRenderTexture(int width, int height)
        {
            width = MathEx.Max(20, width);
            height = MathEx.Max(20, height);

            // Note: Depth buffer and readable flags are required because ScenePicking uses it
            Texture colorTex = Texture.Create2D((uint)width, (uint)height, PixelFormat.RGBA8, TextureUsage.Render | TextureUsage.CPUReadable);
            Texture depthTex = Texture.Create2D((uint)width, (uint)height, PixelFormat.D32_S8X24, TextureUsage.DepthStencil | TextureUsage.CPUReadable);

            renderTexture = new RenderTexture(colorTex, depthTex);
            renderTexture.Priority = 1;

            if (camera == null)
            {
                SceneObject sceneCameraSO = new SceneObject("SceneCamera", true);
                camera = sceneCameraSO.AddComponent<Camera>();
                camera.Viewport.Target = renderTexture;
                camera.Viewport.Area = new Rect2(0.0f, 0.0f, 1.0f, 1.0f);

                sceneCameraSO.Position = new Vector3(0, 0.5f, 1);
                sceneCameraSO.LookAt(new Vector3(0, 0.5f, 0));

                camera.Priority = 2;
                camera.NearClipPlane = 0.05f;
                camera.FarClipPlane = 2500.0f;
                camera.Viewport.ClearColor = ClearColor;
                camera.Layers = UInt64.MaxValue & ~SceneAxesHandle.LAYER; // Don't draw scene axes in this camera

                cameraController = sceneCameraSO.AddComponent<SceneCamera>();

                renderTextureGUI = new GUIRenderTexture(renderTexture);
                rtPanel.AddElement(renderTextureGUI);

                sceneGrid = new SceneGrid(camera);
                sceneSelection = new SceneSelection(camera);
                sceneGizmos = new SceneGizmos(camera);
                sceneHandles = new SceneHandles(this, camera);
            }
            else
            {
                camera.Viewport.Target = renderTexture;
                renderTextureGUI.RenderTexture = renderTexture;
            }

            Rect2I rtBounds = new Rect2I(0, 0, width, height);
            renderTextureGUI.Bounds = rtBounds;
            focusCatcher.Bounds = GUIUtility.CalculateBounds(rtPanel, GUI);

            sceneAxesGUI.SetPosition(width - HandleAxesGUISize - HandleAxesGUIPaddingX, HandleAxesGUIPaddingY);

            // TODO - Consider only doing the resize once user stops resizing the widget in order to reduce constant
            // render target destroy/create cycle for every single pixel.

            camera.AspectRatio = width / (float)height;

            if (profilerCamera != null)
                profilerCamera.Viewport.Target = renderTexture;
        }

        /// <summary>
        /// Parses an array of scene objects and removes elements that are children of elements that are also in the array.
        /// </summary>
        /// <param name="objects">Array containing duplicate objects as input, and array without duplicate objects as
        ///                       output.</param>
        private void CleanDuplicates(ref SceneObject[] objects)
        {
            List<SceneObject> cleanList = new List<SceneObject>();
            for (int i = 0; i < objects.Length; i++)
            {
                bool foundParent = false;
                for (int j = 0; j < objects.Length; j++)
                {
                    SceneObject elem = objects[i];

                    while (elem != null && elem != objects[j])
                        elem = elem.Parent;

                    bool isChildOf = elem == objects[j];

                    if (i != j && isChildOf)
                    {
                        foundParent = true;
                        break;
                    }
                }

                if (!foundParent)
                    cleanList.Add(objects[i]);
            }

            objects = cleanList.ToArray();
        }

        /// <summary>
        /// Starts a drag operation that displays a selection outline allowing the user to select multiple entries at once.
        /// </summary>
        /// <param name="scenePos">Coordinates relative to the scene where the drag originated.</param>
        private void StartDragSelection(Vector2I scenePos)
        {
            isDraggingSelection = true;
            dragSelectionStart = scenePos;
            dragSelectionEnd = dragSelectionStart;
        }

        /// <summary>
        /// Updates a selection outline drag operation by expanding the outline to the new location. Elements in the outline
        /// are selected.
        /// </summary>
        /// <param name="scenePos">Coordinates of the pointer relative to the scene.</param>
        /// <returns>True if the selection outline drag is valid and was updated, false otherwise.</returns>
        private bool UpdateDragSelection(Vector2I scenePos)
        {
            if (!isDraggingSelection)
                return false;

            if (dragSelection == null)
            {
                dragSelection = new GUITexture(null, true, EditorStylesInternal.SelectionArea);
                selectionPanel.AddElement(dragSelection);
            }
            
            dragSelectionEnd = scenePos;

            Rect2I selectionArea = new Rect2I();

            Vector2I min = new Vector2I(Math.Min(dragSelectionStart.x, dragSelectionEnd.x), Math.Min(dragSelectionStart.y, dragSelectionEnd.y));
            Vector2I max = new Vector2I(Math.Max(dragSelectionStart.x, dragSelectionEnd.x), Math.Max(dragSelectionStart.y, dragSelectionEnd.y));
            selectionArea.x = min.x;
            selectionArea.y = min.y;
            selectionArea.width = Math.Max(max.x - min.x, 1);
            selectionArea.height = Math.Max(max.y - min.y, 1);

            dragSelection.Bounds = selectionArea;

            return true;
        }

        /// <summary>
        /// Ends the selection outline drag operation. Elements in the outline are selected.
        /// </summary>
        /// <returns>True if the selection outline drag is valid and was ended, false otherwise.</returns>
        private bool EndDragSelection()
        {
            if (!isDraggingSelection)
                return false;

            if (dragSelection != null)
            {
                dragSelection.Destroy();
                dragSelection = null;
            }

            if ((dragSelectionEnd - dragSelectionStart).Length < 1)
            {
                isDraggingSelection = false;
                return false;
            }

            Vector2I min = new Vector2I(Math.Min(dragSelectionStart.x, dragSelectionEnd.x),
                    Math.Min(dragSelectionStart.y, dragSelectionEnd.y));
            Vector2I max = new Vector2I(Math.Max(dragSelectionStart.x, dragSelectionEnd.x),
                Math.Max(dragSelectionStart.y, dragSelectionEnd.y));
            sceneSelection.PickObjects(min, max - min,
                Input.IsButtonHeld(ButtonCode.LeftControl) || Input.IsButtonHeld(ButtonCode.RightControl));
            isDraggingSelection = false;
            return true;
        }
    }

    /** @} */
}

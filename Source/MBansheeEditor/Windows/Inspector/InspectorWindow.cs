//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Data.Common;
using System.IO;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspector
     *  @{
     */

    /// <summary>
    /// Displays GUI for a <see cref="SceneObject"/> or for a <see cref="Resource"/>. Scene object's transform values
    /// are displayed, along with all their components and their fields.
    /// </summary>
    internal sealed class InspectorWindow : EditorWindow
    {
        /// <summary>
        /// Type of objects displayed in the window.
        /// </summary>
        private enum InspectorType
        {
            SceneObject,
            Resource,
            Multiple,
            None
        }

        /// <summary>
        /// Inspector GUI elements for a single <see cref="Component"/> in a <see cref="SceneObject"/>.
        /// </summary>
        private class InspectorComponent
        {
            public GUIToggle foldout;
            public GUIButton removeBtn;
            public GUILayout title;
            public GUIPanel panel;
            public Inspector inspector;
            public UInt64 instanceId;
        }

        /// <summary>
        /// Inspector GUI elements for a <see cref="Resource"/>
        /// </summary>
        private class InspectorResource
        {
            public GUIPanel panel;
            public Inspector inspector;
        }

        private static readonly Color HIGHLIGHT_COLOR = new Color(1.0f, 1.0f, 1.0f, 0.5f);
        private const int RESOURCE_TITLE_HEIGHT = 30;
        private const int COMPONENT_SPACING = 10;
        private const int PADDING = 5;

        private List<InspectorComponent> inspectorComponents = new List<InspectorComponent>();
        private InspectorPersistentData persistentData;
        private InspectorResource inspectorResource;
        private GUIScrollArea inspectorScrollArea;
        private GUILayout inspectorLayout;
        private GUIPanel highlightPanel;
        private GUITexture scrollAreaHighlight;

        private SceneObject activeSO;
        private InspectableState modifyState;
        private int undoCommandIdx = -1;
        private GUITextBox soNameInput;
        private GUIToggle soActiveToggle;
        private GUILayout soPrefabLayout;
        private bool soHasPrefab;
        private GUIFloatField soPosX;
        private GUIFloatField soPosY;
        private GUIFloatField soPosZ;
        private GUIFloatField soRotX;
        private GUIFloatField soRotY;
        private GUIFloatField soRotZ;
        private GUIFloatField soScaleX;
        private GUIFloatField soScaleY;
        private GUIFloatField soScaleZ;

        private Rect2I[] dropAreas = new Rect2I[0];

        private InspectorType currentType = InspectorType.None;
        private Resource activeResource;

        /// <summary>
        /// Opens the inspector window from the menu bar.
        /// </summary>
        [MenuItem("Windows/Inspector", ButtonModifier.CtrlAlt, ButtonCode.I, 6000)]
        private static void OpenInspectorWindow()
        {
            OpenWindow<InspectorWindow>();
        }

        /// <summary>
        /// Name of the inspector window to display on the window title.
        /// </summary>
        /// <returns>Name of the inspector window to display on the window title.</returns>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Inspector");
        }

        /// <summary>
        /// Sets a resource whose GUI is to be displayed in the inspector. Clears any previous contents of the window.
        /// </summary>
        /// <param name="resourcePath">Resource path relative to the project of the resource to inspect.</param>
        private void SetObjectToInspect(String resourcePath)
        {
            activeResource = ProjectLibrary.Load<Resource>(resourcePath);

            if (activeResource == null)
                return;

            currentType = InspectorType.Resource;

            inspectorScrollArea = new GUIScrollArea();
            GUI.AddElement(inspectorScrollArea);
            inspectorLayout = inspectorScrollArea.Layout;

            GUIPanel titlePanel = inspectorLayout.AddPanel();
            titlePanel.SetHeight(RESOURCE_TITLE_HEIGHT);

            GUILayoutY titleLayout = titlePanel.AddLayoutY();
            titleLayout.SetPosition(PADDING, PADDING);

            string name = Path.GetFileNameWithoutExtension(resourcePath);
            string type = activeResource.GetType().Name;

            LocString title = new LocEdString(name + " (" + type + ")");
            GUILabel titleLabel = new GUILabel(title);

            titleLayout.AddFlexibleSpace();
            GUILayoutX titleLabelLayout = titleLayout.AddLayoutX();
            titleLabelLayout.AddElement(titleLabel);
            titleLayout.AddFlexibleSpace();

            GUIPanel titleBgPanel = titlePanel.AddPanel(1);

            GUITexture titleBg = new GUITexture(null, EditorStyles.InspectorTitleBg);
            titleBgPanel.AddElement(titleBg);

            inspectorLayout.AddSpace(COMPONENT_SPACING);

            inspectorResource = new InspectorResource();
            inspectorResource.panel = inspectorLayout.AddPanel();

            var persistentProperties = persistentData.GetProperties(activeResource.UUID);

            inspectorResource.inspector = InspectorUtility.GetInspector(activeResource.GetType());
            inspectorResource.inspector.Initialize(inspectorResource.panel, activeResource, persistentProperties);

            inspectorLayout.AddFlexibleSpace();
        }

        /// <summary>
        /// Sets a scene object whose GUI is to be displayed in the inspector. Clears any previous contents of the window.
        /// </summary>
        /// <param name="so">Scene object to inspect.</param>
        private void SetObjectToInspect(SceneObject so)
        {
            if (so == null)
                return;

            currentType = InspectorType.SceneObject;
            activeSO = so;

            inspectorScrollArea = new GUIScrollArea();
            scrollAreaHighlight = new GUITexture(Builtin.WhiteTexture);
            scrollAreaHighlight.SetTint(HIGHLIGHT_COLOR);
            scrollAreaHighlight.Active = false;

            GUI.AddElement(inspectorScrollArea);
            GUIPanel inspectorPanel = inspectorScrollArea.Layout.AddPanel();
            inspectorLayout = inspectorPanel.AddLayoutY();
            highlightPanel = inspectorPanel.AddPanel(-1);
            highlightPanel.AddElement(scrollAreaHighlight);

            // SceneObject fields
            CreateSceneObjectFields();
            RefreshSceneObjectFields(true);

            // Components
            Component[] allComponents = so.GetComponents();
            for (int i = 0; i < allComponents.Length; i++)
            {
                inspectorLayout.AddSpace(COMPONENT_SPACING);

                InspectorComponent data = new InspectorComponent();
                data.instanceId = allComponents[i].InstanceId;

                data.foldout = new GUIToggle(allComponents[i].GetType().Name, EditorStyles.Foldout);
                data.removeBtn = new GUIButton(new GUIContent(EditorBuiltin.XBtnIcon), GUIOption.FixedWidth(30));

                data.title = inspectorLayout.AddLayoutX();
                data.title.AddElement(data.foldout);
                data.title.AddElement(data.removeBtn);
                data.panel = inspectorLayout.AddPanel();

                var persistentProperties = persistentData.GetProperties(allComponents[i].InstanceId);

                data.inspector = InspectorUtility.GetInspector(allComponents[i].GetType());
                data.inspector.Initialize(data.panel, allComponents[i], persistentProperties);

                bool isExpanded = data.inspector.Persistent.GetBool(data.instanceId + "_Expanded", true);
                data.foldout.Value = isExpanded;

                if (!isExpanded)
                    data.inspector.SetVisible(false);

                Type curComponentType = allComponents[i].GetType();
                data.foldout.OnToggled += (bool expanded) => OnComponentFoldoutToggled(data, expanded);
                data.removeBtn.OnClick += () => OnComponentRemoveClicked(curComponentType);

                inspectorComponents.Add(data);
            }

            inspectorLayout.AddFlexibleSpace();

            UpdateDropAreas();
        }

        /// <summary>
        /// Creates GUI elements required for displaying <see cref="SceneObject"/> fields like name, prefab data and 
        /// transform (position, rotation, scale). Assumes that necessary inspector scroll area layout has already been 
        /// created.
        /// </summary>
        private void CreateSceneObjectFields()
        {
            GUIPanel sceneObjectPanel = inspectorLayout.AddPanel();
            sceneObjectPanel.SetHeight(GetTitleBounds().height);

            GUILayoutY sceneObjectLayout = sceneObjectPanel.AddLayoutY();
            sceneObjectLayout.SetPosition(PADDING, PADDING);

            GUIPanel sceneObjectBgPanel = sceneObjectPanel.AddPanel(1);

            GUILayoutX nameLayout = sceneObjectLayout.AddLayoutX();
            soActiveToggle = new GUIToggle("");
            soActiveToggle.OnToggled += OnSceneObjectActiveStateToggled;
            GUILabel nameLbl = new GUILabel(new LocEdString("Name"), GUIOption.FixedWidth(50));
            soNameInput = new GUITextBox(false, GUIOption.FlexibleWidth(180));
            soNameInput.Text = activeSO.Name;
            soNameInput.OnChanged += OnSceneObjectRename;
            soNameInput.OnConfirmed += OnModifyConfirm;
            soNameInput.OnFocusLost += OnModifyConfirm;

            nameLayout.AddElement(soActiveToggle);
            nameLayout.AddSpace(3);
            nameLayout.AddElement(nameLbl);
            nameLayout.AddElement(soNameInput);
            nameLayout.AddFlexibleSpace();

            soPrefabLayout = sceneObjectLayout.AddLayoutX();

            GUILayoutX positionLayout = sceneObjectLayout.AddLayoutX();
            GUILabel positionLbl = new GUILabel(new LocEdString("Position"), GUIOption.FixedWidth(50));
            soPosX = new GUIFloatField(new LocEdString("X"), 10, "", GUIOption.FixedWidth(60));
            soPosY = new GUIFloatField(new LocEdString("Y"), 10, "", GUIOption.FixedWidth(60));
            soPosZ = new GUIFloatField(new LocEdString("Z"), 10, "", GUIOption.FixedWidth(60));

            soPosX.OnChanged += (x) => OnPositionChanged(0, x);
            soPosY.OnChanged += (y) => OnPositionChanged(1, y);
            soPosZ.OnChanged += (z) => OnPositionChanged(2, z);

            soPosX.OnConfirmed += OnModifyConfirm;
            soPosY.OnConfirmed += OnModifyConfirm;
            soPosZ.OnConfirmed += OnModifyConfirm;

            soPosX.OnFocusLost += OnModifyConfirm;
            soPosY.OnFocusLost += OnModifyConfirm;
            soPosZ.OnFocusLost += OnModifyConfirm;

            positionLayout.AddElement(positionLbl);
            positionLayout.AddElement(soPosX);
            positionLayout.AddSpace(10);
            positionLayout.AddFlexibleSpace();
            positionLayout.AddElement(soPosY);
            positionLayout.AddSpace(10);
            positionLayout.AddFlexibleSpace();
            positionLayout.AddElement(soPosZ);
            positionLayout.AddFlexibleSpace();

            GUILayoutX rotationLayout = sceneObjectLayout.AddLayoutX();
            GUILabel rotationLbl = new GUILabel(new LocEdString("Rotation"), GUIOption.FixedWidth(50));
            soRotX = new GUIFloatField(new LocEdString("X"), 10, "", GUIOption.FixedWidth(60));
            soRotY = new GUIFloatField(new LocEdString("Y"), 10, "", GUIOption.FixedWidth(60));
            soRotZ = new GUIFloatField(new LocEdString("Z"), 10, "", GUIOption.FixedWidth(60));

            soRotX.OnChanged += (x) => OnRotationChanged(0, x);
            soRotY.OnChanged += (y) => OnRotationChanged(1, y);
            soRotZ.OnChanged += (z) => OnRotationChanged(2, z);

            soRotX.OnConfirmed += OnModifyConfirm;
            soRotY.OnConfirmed += OnModifyConfirm;
            soRotZ.OnConfirmed += OnModifyConfirm;

            soRotX.OnFocusLost += OnModifyConfirm;
            soRotY.OnFocusLost += OnModifyConfirm;
            soRotZ.OnFocusLost += OnModifyConfirm;

            rotationLayout.AddElement(rotationLbl);
            rotationLayout.AddElement(soRotX);
            rotationLayout.AddSpace(10);
            rotationLayout.AddFlexibleSpace();
            rotationLayout.AddElement(soRotY);
            rotationLayout.AddSpace(10);
            rotationLayout.AddFlexibleSpace();
            rotationLayout.AddElement(soRotZ);
            rotationLayout.AddFlexibleSpace();

            GUILayoutX scaleLayout = sceneObjectLayout.AddLayoutX();
            GUILabel scaleLbl = new GUILabel(new LocEdString("Scale"), GUIOption.FixedWidth(50));
            soScaleX = new GUIFloatField(new LocEdString("X"), 10, "", GUIOption.FixedWidth(60));
            soScaleY = new GUIFloatField(new LocEdString("Y"), 10, "", GUIOption.FixedWidth(60));
            soScaleZ = new GUIFloatField(new LocEdString("Z"), 10, "", GUIOption.FixedWidth(60));

            soScaleX.OnChanged += (x) => OnScaleChanged(0, x);
            soScaleY.OnChanged += (y) => OnScaleChanged(1, y);
            soScaleZ.OnChanged += (z) => OnScaleChanged(2, z);

            soScaleX.OnConfirmed += OnModifyConfirm;
            soScaleY.OnConfirmed += OnModifyConfirm;
            soScaleZ.OnConfirmed += OnModifyConfirm;

            soScaleX.OnFocusLost += OnModifyConfirm;
            soScaleY.OnFocusLost += OnModifyConfirm;
            soScaleZ.OnFocusLost += OnModifyConfirm;

            scaleLayout.AddElement(scaleLbl);
            scaleLayout.AddElement(soScaleX);
            scaleLayout.AddSpace(10);
            scaleLayout.AddFlexibleSpace();
            scaleLayout.AddElement(soScaleY);
            scaleLayout.AddSpace(10);
            scaleLayout.AddFlexibleSpace();
            scaleLayout.AddElement(soScaleZ);
            scaleLayout.AddFlexibleSpace();

            sceneObjectLayout.AddFlexibleSpace();

            GUITexture titleBg = new GUITexture(null, EditorStyles.InspectorTitleBg);
            sceneObjectBgPanel.AddElement(titleBg);
        }

        /// <summary>
        /// Updates contents of the scene object specific fields (name, position, rotation, etc.)
        /// </summary>
        /// <param name="forceUpdate">If true, the GUI elements will be updated regardless of whether a change was
        ///                           detected or not.</param>
        private void RefreshSceneObjectFields(bool forceUpdate)
        {
            if (activeSO == null)
                return;

            soNameInput.Text = activeSO.Name;
            soActiveToggle.Value = activeSO.Active;

            SceneObject prefabParent = PrefabUtility.GetPrefabParent(activeSO);

            // Ignore prefab parent if scene root, we only care for non-root prefab instances
            bool hasPrefab = prefabParent != null && prefabParent.Parent != null;
            if (soHasPrefab != hasPrefab || forceUpdate)
            {
                int numChildren = soPrefabLayout.ChildCount;
                for (int i = 0; i < numChildren; i++)
                    soPrefabLayout.GetChild(0).Destroy();

                GUILabel prefabLabel =new GUILabel(new LocEdString("Prefab"), GUIOption.FixedWidth(50));
                soPrefabLayout.AddElement(prefabLabel);

                if (hasPrefab)
                {
                    GUIButton btnApplyPrefab = new GUIButton(new LocEdString("Apply"), GUIOption.FixedWidth(60));
                    GUIButton btnRevertPrefab = new GUIButton(new LocEdString("Revert"), GUIOption.FixedWidth(60));
                    GUIButton btnBreakPrefab = new GUIButton(new LocEdString("Break"), GUIOption.FixedWidth(60));

                    btnApplyPrefab.OnClick += () =>
                    {
                        PrefabUtility.ApplyPrefab(activeSO);

                        // Refresh any prefab instances in the scene
                        PrefabUtility.UpdateFromPrefab(Scene.Root);
                    };
                    btnRevertPrefab.OnClick += () =>
                    {
                        UndoRedo.RecordSO(activeSO, true, "Reverting \"" + activeSO.Name + "\" to prefab.");

                        PrefabUtility.RevertPrefab(activeSO);
                        EditorApplication.SetSceneDirty();
                    };
                    btnBreakPrefab.OnClick += () =>
                    {
                        UndoRedo.BreakPrefab(activeSO, "Breaking prefab link for " + activeSO.Name);

                        EditorApplication.SetSceneDirty();
                    };

                    soPrefabLayout.AddElement(btnApplyPrefab);
                    soPrefabLayout.AddElement(btnRevertPrefab);
                    soPrefabLayout.AddElement(btnBreakPrefab);
                }
                else
                {
                    GUILabel noPrefabLabel = new GUILabel("None");
                    soPrefabLayout.AddElement(noPrefabLabel);
                }

                soHasPrefab = hasPrefab;
            }

            Vector3 position;
            Vector3 angles;
            if (EditorApplication.ActiveCoordinateMode == HandleCoordinateMode.World)
            {
                position = activeSO.Position;
                angles = activeSO.Rotation.ToEuler();
            }
            else
            {
                position = activeSO.LocalPosition;
                angles = activeSO.LocalRotation.ToEuler();
            }

            Vector3 scale = activeSO.LocalScale;

            if(!soPosX.HasInputFocus)
                soPosX.Value = position.x;

            if (!soPosY.HasInputFocus)
                soPosY.Value = position.y;

            if (!soPosZ.HasInputFocus)
                soPosZ.Value = position.z;

            if (!soRotX.HasInputFocus)
                soRotX.Value = angles.x;

            if (!soRotY.HasInputFocus)
                soRotY.Value = angles.y;

            if (!soRotZ.HasInputFocus)
                soRotZ.Value = angles.z;

            if (!soScaleX.HasInputFocus)
                soScaleX.Value = scale.x;

            if (!soScaleY.HasInputFocus)
                soScaleY.Value = scale.y;

            if (!soScaleZ.HasInputFocus)
                soScaleZ.Value = scale.z;
        }

        private void OnInitialize()
        {
            Selection.OnSelectionChanged += OnSelectionChanged;

            const string soName = "InspectorPersistentData";
            SceneObject so = Scene.Root.FindChild(soName);
            if (so == null)
                so = new SceneObject(soName, true);

            persistentData = so.GetComponent<InspectorPersistentData>();
            if (persistentData == null)
                persistentData = so.AddComponent<InspectorPersistentData>();

            OnSelectionChanged(new SceneObject[0], new string[0]);
        }

        private void OnDestroy()
        {
            Selection.OnSelectionChanged -= OnSelectionChanged;
        }

        private void OnEditorUpdate()
        {
            if (currentType == InspectorType.SceneObject)
            {
                Component[] allComponents = activeSO.GetComponents();
                bool requiresRebuild = allComponents.Length != inspectorComponents.Count;

                if (!requiresRebuild)
                {
                    for (int i = 0; i < inspectorComponents.Count; i++)
                    {
                        if (inspectorComponents[i].instanceId != allComponents[i].InstanceId)
                        {
                            requiresRebuild = true;
                            break;
                        }
                    }
                }

                if (requiresRebuild)
                {
                    SceneObject so = activeSO;
                    Clear();
                    SetObjectToInspect(so);
                }
                else
                {
                    RefreshSceneObjectFields(false);

                    InspectableState componentModifyState = InspectableState.NotModified;
                    for (int i = 0; i < inspectorComponents.Count; i++)
                        componentModifyState |= inspectorComponents[i].inspector.Refresh();

                    if (componentModifyState.HasFlag(InspectableState.ModifyInProgress))
                        EditorApplication.SetSceneDirty();

                    modifyState |= componentModifyState;
                }
            }
            else if (currentType == InspectorType.Resource)
            {
                inspectorResource.inspector.Refresh();
            }

            // Detect drag and drop
            bool isValidDrag = false;

            if (activeSO != null)
            {
                if ((DragDrop.DragInProgress || DragDrop.DropInProgress) && DragDrop.Type == DragDropType.Resource)
                {
                    Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);
                    Vector2I scrollPos = windowPos;
                    Rect2I contentBounds = inspectorLayout.Bounds;
                    scrollPos.x -= contentBounds.x;
                    scrollPos.y -= contentBounds.y;

                    bool isInBounds = false;
                    Rect2I dropArea = new Rect2I();
                    foreach (var bounds in dropAreas)
                    {
                        if (bounds.Contains(scrollPos))
                        {
                            isInBounds = true;
                            dropArea = bounds;
                            break;
                        }
                    }

                    Type draggedComponentType = null;
                    if (isInBounds)
                    {
                        ResourceDragDropData dragData = DragDrop.Data as ResourceDragDropData;
                        if (dragData != null)
                        {
                            foreach (var resPath in dragData.Paths)
                            {
                                ResourceMeta meta = ProjectLibrary.GetMeta(resPath);
                                if (meta != null)
                                {
                                    if (meta.ResType == ResourceType.ScriptCode)
                                    {
                                        ScriptCode scriptFile = ProjectLibrary.Load<ScriptCode>(resPath);

                                        if (scriptFile != null)
                                        {
                                            Type[] scriptTypes = scriptFile.Types;
                                            foreach (var type in scriptTypes)
                                            {
                                                if (type.IsSubclassOf(typeof (Component)))
                                                {
                                                    draggedComponentType = type;
                                                    isValidDrag = true;
                                                    break;
                                                }
                                            }

                                            if (draggedComponentType != null)
                                                break;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (isValidDrag)
                    {
                        scrollAreaHighlight.Bounds = dropArea;

                        if (DragDrop.DropInProgress)
                        {
                            activeSO.AddComponent(draggedComponentType);

                            modifyState = InspectableState.Modified;
                            EditorApplication.SetSceneDirty();
                        }
                    }
                }               
            }

            if (scrollAreaHighlight != null)
                scrollAreaHighlight.Active = isValidDrag;
        }

        /// <summary>
        /// Triggered when the user selects a new resource or a scene object, or deselects everything.
        /// </summary>
        /// <param name="objects">A set of new scene objects that were selected.</param>
        /// <param name="paths">A set of absolute resource paths that were selected.</param>
        private void OnSelectionChanged(SceneObject[] objects, string[] paths)
        {
            if (currentType == InspectorType.SceneObject && modifyState == InspectableState.NotModified)
                UndoRedo.PopCommand(undoCommandIdx);

            Clear();
            modifyState = InspectableState.NotModified;

            if (objects.Length == 0 && paths.Length == 0)
            {
                currentType = InspectorType.None;
                inspectorScrollArea = new GUIScrollArea();
                GUI.AddElement(inspectorScrollArea);
                inspectorLayout = inspectorScrollArea.Layout;

                inspectorLayout.AddFlexibleSpace();
                GUILayoutX layoutMsg = inspectorLayout.AddLayoutX();
                layoutMsg.AddFlexibleSpace();
                layoutMsg.AddElement(new GUILabel(new LocEdString("No object selected")));
                layoutMsg.AddFlexibleSpace();
                inspectorLayout.AddFlexibleSpace();
            }
            else if ((objects.Length + paths.Length) > 1)
            {
                currentType = InspectorType.None;
                inspectorScrollArea = new GUIScrollArea();
                GUI.AddElement(inspectorScrollArea);
                inspectorLayout = inspectorScrollArea.Layout;

                inspectorLayout.AddFlexibleSpace();
                GUILayoutX layoutMsg = inspectorLayout.AddLayoutX();
                layoutMsg.AddFlexibleSpace();
                layoutMsg.AddElement(new GUILabel(new LocEdString("Multiple objects selected")));
                layoutMsg.AddFlexibleSpace();
                inspectorLayout.AddFlexibleSpace();
            }
            else if (objects.Length == 1)
            {
                if (objects[0] != null)
                {
                    UndoRedo.RecordSO(objects[0]);
                    undoCommandIdx = UndoRedo.TopCommandId;

                    SetObjectToInspect(objects[0]);
                }
            }
            else if (paths.Length == 1)
            {
                SetObjectToInspect(paths[0]);
            }
        }

        /// <summary>
        /// Triggered when the user closes or expands a component foldout, making the component fields visible or hidden.
        /// </summary>
        /// <param name="inspectorData">Contains GUI data for the component that was toggled.</param>
        /// <param name="expanded">Determines whether to display or hide component contents.</param>
        private void OnComponentFoldoutToggled(InspectorComponent inspectorData, bool expanded)
        {
            inspectorData.inspector.Persistent.SetBool(inspectorData.instanceId + "_Expanded", expanded);
            inspectorData.inspector.SetVisible(expanded);
        }

        /// <summary>
        /// Triggered when the user clicks the component remove button. Removes that component from the active scene object.
        /// </summary>
        /// <param name="componentType">Type of the component to remove.</param>
        private void OnComponentRemoveClicked(Type componentType)
        {
            if (activeSO != null)
            {
                activeSO.RemoveComponent(componentType);

                modifyState = InspectableState.Modified;
                EditorApplication.SetSceneDirty();
            }
        }

        /// <summary>
        /// Destroys all inspector GUI elements.
        /// </summary>
        internal void Clear()
        {
            for (int i = 0; i < inspectorComponents.Count; i++)
            {
                inspectorComponents[i].foldout.Destroy();
                inspectorComponents[i].removeBtn.Destroy();
                inspectorComponents[i].inspector.Destroy();
            }

            inspectorComponents.Clear();

            if (inspectorResource != null)
            {
                inspectorResource.inspector.Destroy();
                inspectorResource = null;
            }

            if (inspectorScrollArea != null)
            {
                inspectorScrollArea.Destroy();
                inspectorScrollArea = null;
            }

            if (scrollAreaHighlight != null)
            {
                scrollAreaHighlight.Destroy();
                scrollAreaHighlight = null;
            }

            if (highlightPanel != null)
            {
                highlightPanel.Destroy();
                highlightPanel = null;
            }

            activeSO = null;
            soNameInput = null;
            soActiveToggle = null;
            soPrefabLayout = null;
            soHasPrefab = false;
            soPosX = null;
            soPosY = null;
            soPosZ = null;
            soRotX = null;
            soRotY = null;
            soRotZ = null;
            soScaleX = null;
            soScaleY = null;
            soScaleZ = null;
            dropAreas = new Rect2I[0];

            activeResource = null;
            currentType = InspectorType.None;
        }

        /// <summary>
        /// Returns the size of the title bar area that is displayed for <see cref="SceneObject"/> specific fields.
        /// </summary>
        /// <returns>Area of the title bar, relative to the window.</returns>
        private Rect2I GetTitleBounds()
        {
            return new Rect2I(0, 0, Width, 115);
        }

        /// <summary>
        /// Triggered when the user changes the name of the currently active scene object.
        /// </summary>
        private void OnSceneObjectRename(string name)
        {
            if (activeSO != null)
            {
                activeSO.Name = name;

                modifyState |= InspectableState.ModifyInProgress;
                EditorApplication.SetSceneDirty();
            }
        }

        /// <summary>
        /// Triggered when the user changes the active state of the scene object.
        /// </summary>
        /// <param name="active">True if the object is active, false otherwise.</param>
        private void OnSceneObjectActiveStateToggled(bool active)
        {
            if (activeSO != null)
                activeSO.Active = active;
        }

        /// <summary>
        /// Triggered when the scene object modification is confirmed by the user.
        /// </summary>
        private void OnModifyConfirm()
        {
            if (modifyState.HasFlag(InspectableState.ModifyInProgress))
                modifyState = InspectableState.Modified;
        }

        /// <summary>
        /// Triggered when the position value in the currently active <see cref="SceneObject"/> changes. Updates the 
        /// necessary GUI elements.
        /// </summary>
        /// <param name="idx">Index of the coordinate that was changed.</param>
        /// <param name="value">New value of the field.</param>
        private void OnPositionChanged(int idx, float value)
        {
            if (activeSO == null)
                return;

            if (EditorApplication.ActiveCoordinateMode == HandleCoordinateMode.World)
            {
                Vector3 position = activeSO.Position;
                position[idx] = value;
                activeSO.Position = position;
            }
            else
            {
                Vector3 position = activeSO.LocalPosition;
                position[idx] = value;
                activeSO.LocalPosition = position;
            }

            modifyState = InspectableState.ModifyInProgress;
            EditorApplication.SetSceneDirty();
        }

        /// <summary>
        /// Triggered when the rotation value in the currently active <see cref="SceneObject"/> changes. Updates the 
        /// necessary GUI elements.
        /// </summary>
        /// <param name="idx">Index of the euler angle that was changed (0 - X, 1 - Y, 2 - Z).</param>
        /// <param name="value">New value of the field.</param>
        private void OnRotationChanged(int idx, float value)
        {
            if (activeSO == null)
                return;

            if (EditorApplication.ActiveCoordinateMode == HandleCoordinateMode.World)
            {
                Vector3 angles = activeSO.Rotation.ToEuler();
                angles[idx] = value;
                activeSO.Rotation = Quaternion.FromEuler(angles);
            }
            else
            {
                Vector3 angles = activeSO.LocalRotation.ToEuler();
                angles[idx] = value;
                activeSO.LocalRotation = Quaternion.FromEuler(angles);
            }

            modifyState = InspectableState.ModifyInProgress;
            EditorApplication.SetSceneDirty();
        }

        /// <summary>
        /// Triggered when the scale value in the currently active <see cref="SceneObject"/> changes. Updates the 
        /// necessary GUI elements.
        /// </summary>
        /// <param name="idx">Index of the coordinate that was changed.</param>
        /// <param name="value">New value of the field.</param>
        private void OnScaleChanged(int idx, float value)
        {
            if (activeSO == null)
                return;

            Vector3 scale = activeSO.LocalScale;
            scale[idx] = value;
            activeSO.LocalScale = scale;

            modifyState = InspectableState.ModifyInProgress;
            EditorApplication.SetSceneDirty();
        }

        /// <inheritdoc/>
        protected override void WindowResized(int width, int height)
        {
            base.WindowResized(width, height);

            UpdateDropAreas();
        }

        /// <summary>
        /// Updates drop areas used for dragging and dropping components on the inspector.
        /// </summary>
        private void UpdateDropAreas()
        {
            if (activeSO == null)
                return;

            Rect2I contentBounds = inspectorLayout.Bounds;
            dropAreas = new Rect2I[inspectorComponents.Count + 1];
            int yOffset = GetTitleBounds().height;
            for (int i = 0; i < inspectorComponents.Count; i++)
            {
                dropAreas[i] = new Rect2I(0, yOffset, contentBounds.width, COMPONENT_SPACING);
                yOffset += inspectorComponents[i].title.Bounds.height + inspectorComponents[i].panel.Bounds.height + COMPONENT_SPACING;
            }

            dropAreas[dropAreas.Length - 1] = new Rect2I(0, yOffset, contentBounds.width, contentBounds.height - yOffset);
        }
    }

    /** @} */
}

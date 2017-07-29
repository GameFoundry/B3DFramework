//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="Camera"/> component.
    /// </summary>
    [CustomInspector(typeof(Camera))]
    internal class CameraInspector : Inspector
    {
        private GUIEnumField projectionTypeField = new GUIEnumField(typeof(ProjectionType), new LocEdString("Projection type"));
        private GUISliderField fieldOfView = new GUISliderField(1, 360, new LocEdString("Field of view"));
        private GUIFloatField orthoHeight = new GUIFloatField(new LocEdString("Orthographic height"));
        private GUIFloatField aspectField = new GUIFloatField(new LocEdString("Aspect ratio"));
        private GUIFloatField nearPlaneField = new GUIFloatField(new LocEdString("Near plane"));
        private GUIFloatField farPlaneField = new GUIFloatField(new LocEdString("Far plane"));
        private GUIFloatField viewportXField = new GUIFloatField(new LocEdString("X"), 30);
        private GUIFloatField viewportYField = new GUIFloatField(new LocEdString("Y"), 30);
        private GUIFloatField viewportWidthField = new GUIFloatField(new LocEdString("Width"), 30);
        private GUIFloatField viewportHeightField = new GUIFloatField(new LocEdString("Height"), 30);
        private GUIEnumField clearFlagsFields = new GUIEnumField(typeof (ClearFlags), true, new LocEdString("Clear flags"));
        private GUIIntField clearStencilField = new GUIIntField(new LocEdString("Clear stencil"));
        private GUIFloatField clearDepthField = new GUIFloatField(new LocEdString("Clear depth"));
        private GUIColorField clearColorField = new GUIColorField(new LocEdString("Clear color"));
        private GUIIntField priorityField = new GUIIntField(new LocEdString("Render priority"));
        private GUIListBoxField layersField = new GUIListBoxField(Layers.Names, true, new LocEdString("Layers"));
        private GUIToggleField mainField = new GUIToggleField(new LocEdString("Main"));

        private GUIToggle renderSettingsFoldout = new GUIToggle(new LocEdString("Render settings"), EditorStyles.Foldout);
        private RenderSettingsGUI renderSettingsGUI;
        private GUILayout renderSettingsLayout;

        private ulong layersValue = 0;
        private InspectableState modifyState;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            BuildGUI();
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            Camera camera = InspectedObject as Camera;
            if (camera == null)
                return InspectableState.NotModified;

            ProjectionType projType = camera.ProjectionType;
            if (projectionTypeField.Value != (ulong)projType)
            {
                projectionTypeField.Value = (ulong)projType;
                ToggleTypeSpecificFields(projType);
            }

            fieldOfView.Value = camera.FieldOfView.Degrees;
            orthoHeight.Value = camera.OrthoHeight;
            aspectField.Value = camera.AspectRatio;
            nearPlaneField.Value = camera.NearClipPlane;
            farPlaneField.Value = camera.FarClipPlane;
            viewportXField.Value = camera.ViewportRect.x;
            viewportYField.Value = camera.ViewportRect.y;
            viewportWidthField.Value = camera.ViewportRect.width;
            viewportHeightField.Value = camera.ViewportRect.height;
            clearFlagsFields.Value = (ulong)camera.ClearFlags;
            clearStencilField.Value = camera.ClearStencil;
            clearDepthField.Value = camera.ClearDepth;
            clearColorField.Value = camera.ClearColor;
            priorityField.Value = camera.Priority;
            mainField.Value = camera.Main;
            renderSettingsGUI.Settings = camera.RenderSettings;

            if (layersValue != camera.Layers)
            {
                bool[] states = new bool[64];
                for (int i = 0; i < states.Length; i++)
                    states[i] = (camera.Layers & Layers.Values[i]) == Layers.Values[i];

                layersField.States = states;
                layersValue = camera.Layers;
            }

            InspectableState oldState = modifyState;
            if (modifyState.HasFlag(InspectableState.Modified))
                modifyState = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Recreates all the GUI elements used by this inspector.
        /// </summary>
        private void BuildGUI()
        {
            if (InspectedObject != null)
            {
                Camera camera = (Camera)InspectedObject;

                projectionTypeField.OnSelectionChanged += x =>
                {
                    camera.ProjectionType = (ProjectionType)x;
                    MarkAsModified();
                    ConfirmModify();
                    ToggleTypeSpecificFields((ProjectionType)x);
                };

                fieldOfView.OnChanged += x => { camera.FieldOfView = (Degree)x; MarkAsModified(); };
                fieldOfView.OnFocusLost += ConfirmModify;

                orthoHeight.OnChanged += x => { camera.OrthoHeight = x; MarkAsModified(); };
                orthoHeight.OnConfirmed += ConfirmModify;
                orthoHeight.OnFocusLost += ConfirmModify;

                aspectField.OnChanged += x => { camera.AspectRatio = x; MarkAsModified(); };
                aspectField.OnConfirmed += ConfirmModify;
                aspectField.OnFocusLost += ConfirmModify;

                nearPlaneField.OnChanged += x => { camera.NearClipPlane = x; MarkAsModified(); };
                nearPlaneField.OnConfirmed += ConfirmModify;
                nearPlaneField.OnFocusLost += ConfirmModify;

                farPlaneField.OnChanged += x => { camera.FarClipPlane = x; MarkAsModified(); };
                farPlaneField.OnConfirmed += ConfirmModify;
                farPlaneField.OnFocusLost += ConfirmModify;

                viewportXField.OnChanged += x =>
                {
                    Rect2 rect = camera.ViewportRect; 
                    rect.x = x; 
                    camera.ViewportRect = rect;

                    MarkAsModified();
                };
                viewportXField.OnConfirmed += ConfirmModify;
                viewportXField.OnFocusLost += ConfirmModify;

                viewportYField.OnChanged += x =>
                {
                    Rect2 rect = camera.ViewportRect; 
                    rect.y = x; 
                    camera.ViewportRect = rect;

                    MarkAsModified();
                };
                viewportYField.OnConfirmed += ConfirmModify;
                viewportYField.OnFocusLost += ConfirmModify;

                viewportWidthField.OnChanged += x =>
                {
                    Rect2 rect = camera.ViewportRect; 
                    rect.width = x; 
                    camera.ViewportRect = rect;

                    MarkAsModified();
                };
                viewportWidthField.OnConfirmed += ConfirmModify;
                viewportWidthField.OnFocusLost += ConfirmModify;

                viewportHeightField.OnChanged += x =>
                {
                    Rect2 rect = camera.ViewportRect; 
                    rect.height = x; 
                    camera.ViewportRect = rect;

                    MarkAsModified();
                };
                viewportHeightField.OnConfirmed += ConfirmModify;
                viewportHeightField.OnFocusLost += ConfirmModify;

                clearFlagsFields.OnSelectionChanged += x =>
                {
                    camera.ClearFlags = (ClearFlags) x;
                    MarkAsModified();
                    ConfirmModify();
                };

                clearStencilField.OnChanged += x => { camera.ClearStencil = (ushort) x; };
                clearStencilField.OnConfirmed += ConfirmModify;
                clearStencilField.OnFocusLost += ConfirmModify;

                clearDepthField.OnChanged += x => { camera.ClearDepth = x; };
                clearDepthField.OnConfirmed += ConfirmModify;
                clearDepthField.OnFocusLost += ConfirmModify;

                clearColorField.OnChanged += x =>
                {
                    camera.ClearColor = x;
                    MarkAsModified();
                    ConfirmModify();
                };

                priorityField.OnChanged += x => { camera.Priority = x; MarkAsModified(); };
                priorityField.OnConfirmed += ConfirmModify;
                priorityField.OnFocusLost += ConfirmModify;

                layersField.OnSelectionChanged += x =>
                {
                    ulong layers = 0;
                    bool[] states = layersField.States;
                    for (int i = 0; i < states.Length; i++)
                        layers |= states[i] ? Layers.Values[i] : 0;

                    layersValue = layers;
                    camera.Layers = layers;

                    MarkAsModified();
                    ConfirmModify();
                };

                mainField.OnChanged += x =>
                {
                    camera.Main = x; 
                    MarkAsModified();
                    ConfirmModify();
                };

                Layout.AddElement(projectionTypeField);
                Layout.AddElement(fieldOfView);
                Layout.AddElement(orthoHeight);
                Layout.AddElement(aspectField);
                Layout.AddElement(nearPlaneField);
                Layout.AddElement(farPlaneField);
                GUILayoutX viewportTopLayout = Layout.AddLayoutX();
                viewportTopLayout.AddElement(new GUILabel(new LocEdString("Viewport"), GUIOption.FixedWidth(100)));
                GUILayoutY viewportContentLayout = viewportTopLayout.AddLayoutY();

                GUILayoutX viewportTopRow = viewportContentLayout.AddLayoutX();
                viewportTopRow.AddElement(viewportXField);
                viewportTopRow.AddElement(viewportWidthField);

                GUILayoutX viewportBotRow = viewportContentLayout.AddLayoutX();
                viewportBotRow.AddElement(viewportYField);
                viewportBotRow.AddElement(viewportHeightField);

                Layout.AddElement(clearFlagsFields);
                Layout.AddElement(clearColorField);
                Layout.AddElement(clearDepthField);
                Layout.AddElement(clearStencilField);
                Layout.AddElement(priorityField);
                Layout.AddElement(layersField);
                Layout.AddElement(mainField);

                renderSettingsFoldout.OnToggled += x =>
                {
                    Persistent.SetBool("renderSettings_Expanded", x);
                    renderSettingsLayout.Active = x;
                };
                Layout.AddElement(renderSettingsFoldout);

                renderSettingsLayout = Layout.AddLayoutX();
                {
                    renderSettingsLayout.AddSpace(10);

                    GUILayoutY contentsLayout = renderSettingsLayout.AddLayoutY();
                    renderSettingsGUI = new RenderSettingsGUI(camera.RenderSettings, contentsLayout, Persistent);
                    renderSettingsGUI.OnChanged += x => { camera.RenderSettings = x; MarkAsModified(); };
                    renderSettingsGUI.OnConfirmed += ConfirmModify;
                }

                ToggleTypeSpecificFields(camera.ProjectionType);
                renderSettingsLayout.Active = Persistent.GetBool("renderSettings_Expanded");
            }
        }

        /// <summary>
        /// Enables or disables different GUI elements depending on the projection type.
        /// </summary>
        /// <param name="type">Projection type to show GUI elements for.</param>
        private void ToggleTypeSpecificFields(ProjectionType type)
        {
            if (type == ProjectionType.Orthographic)
            {
                fieldOfView.Active = false;
                orthoHeight.Active = true;
            }
            else
            {
                fieldOfView.Active = true;
                orthoHeight.Active = false;
            }
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        protected void MarkAsModified()
        {
            modifyState |= InspectableState.ModifyInProgress;
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        protected void ConfirmModify()
        {
            if (modifyState.HasFlag(InspectableState.ModifyInProgress))
                modifyState |= InspectableState.Modified;
        }
    }

    /** @} */
}

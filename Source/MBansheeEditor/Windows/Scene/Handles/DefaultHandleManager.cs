//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Handles
     *  @{
     */

    /// <summary>
    /// Manages the default set of built-in handles like move, rotate and scale. Handles switching between the three handle
    /// types, as well as switching between coordinate and pivot modes, while tracking and updating active scene object
    /// selection. Essentially it detects which handles need to be displayed and where, and then forwards the data
    /// to the specific handle for processing.
    /// </summary>
    internal sealed class DefaultHandleManager : Handle
    {
        /// <summary>
        /// Representation of a scene object currently being modified by a handle.
        /// </summary>
        private struct HandledObject
        {
            /// <summary>
            /// Creates a new scene object representation by recording the current object transform.
            /// </summary>
            /// <param name="so">Scene object that is being modified by a handle.</param>
            public HandledObject(SceneObject so)
            {
                this.so = so;
                initialPosition = so.LocalPosition;
                initialRotation = so.LocalRotation;
                initialScale = so.LocalScale;
            }

            public SceneObject so;
            public Vector3 initialPosition;
            public Quaternion initialRotation;
            public Vector3 initialScale;
        }

        private SceneViewTool activeHandleType = SceneViewTool.View;
        private DefaultHandle activeHandle;

        private HandledObject[] activeSelection;
        private bool isDragged;
        private Vector3 initialHandlePosition;
        private Quaternion initialHandleRotation;

        /// <inheritdoc/>
        protected internal override void PreInput()
        {
            SceneObject[] selectedSceneObjects = Selection.SceneObjects;

            if (selectedSceneObjects.Length == 0)
            {
                if (activeHandle != null)
                {
                    activeHandle.Destroy();
                    activeHandle = null;
                }
            }
            else
            {
                if (activeHandleType != EditorApplication.ActiveSceneTool || activeHandle == null)
                {
                    if (activeHandle != null)
                    {
                        activeHandle.Destroy();
                        activeHandle = null;
                    }

                    switch (EditorApplication.ActiveSceneTool)
                    {
                        case SceneViewTool.Move:
                            activeHandle = new MoveHandle();
                            break;
                        case SceneViewTool.Rotate:
                            activeHandle = new RotateHandle();
                            break;
                        case SceneViewTool.Scale:
                            activeHandle = new ScaleHandle();
                            break;
                    }

                    activeHandleType = EditorApplication.ActiveSceneTool;
                }
            }

            if (activeHandle != null)
            {
                // In case the object moved programmatically, make the handle reflect its current transform
                UpdateActiveHandleTransform(selectedSceneObjects);

                activeHandle.PreInput();
            }
        }

        /// <inheritdoc/>
        protected internal override void PostInput()
        {
            if (activeHandle != null)
            {
                if (activeHandle.IsDragged())
                {
                    if (!isDragged)
                    {
                        isDragged = true;

                        SceneObject[] selectedSceneObjects = Selection.SceneObjects;
                        activeSelection = new HandledObject[selectedSceneObjects.Length];
                        for (int i = 0; i < selectedSceneObjects.Length; i++)
                            activeSelection[i] = new HandledObject(selectedSceneObjects[i]);

                        initialHandlePosition = activeHandle.Position;
                        initialHandleRotation = activeHandle.Rotation;
                    }
                }
                else
                {
                    isDragged = false;
                    activeSelection = null;
                }

                activeHandle.PostInput();

                if (activeHandle.IsDragged())
                {
                    switch (activeHandleType)
                    {
                        case SceneViewTool.Move:
                            MoveHandle moveHandle = (MoveHandle) activeHandle;

                            foreach (var selectedObj in activeSelection)
                            {
                                SceneObject parentSO = selectedObj.so.Parent;
                                if (parentSO == null)
                                    selectedObj.so.LocalPosition = selectedObj.initialPosition + moveHandle.Delta;
                                else
                                {
                                    Vector3 parentRelativeDelta = parentSO.Rotation.Inverse.Rotate(moveHandle.Delta);
                                    selectedObj.so.LocalPosition = selectedObj.initialPosition + parentRelativeDelta;
                                }
                            }

                            break;
                        case SceneViewTool.Rotate:
                        {
                            RotateHandle rotateHandle = (RotateHandle) activeHandle;

                            // Make sure we transform relative to the handle position
                            SceneObject temporarySO = new SceneObject("Temp");
                            temporarySO.Position = initialHandlePosition;
                            temporarySO.LocalRotation = initialHandleRotation;

                            SceneObject[] originalParents = new SceneObject[activeSelection.Length];
                            for (int i = 0; i < activeSelection.Length; i++)
                            {
                                originalParents[i] = activeSelection[i].so.Parent;
                                activeSelection[i].so.LocalPosition = activeSelection[i].initialPosition;
                                activeSelection[i].so.LocalRotation = activeSelection[i].initialRotation;
                                activeSelection[i].so.Parent = temporarySO;
                            }

                            temporarySO.LocalRotation *= rotateHandle.Delta;

                            for (int i = 0; i < activeSelection.Length; i++)
                                activeSelection[i].so.Parent = originalParents[i];

                            temporarySO.Destroy();
                        }
                            break;
                        case SceneViewTool.Scale:
                        {
                            ScaleHandle scaleHandle = (ScaleHandle) activeHandle;

                            // Make sure we transform relative to the handle position
                            SceneObject temporarySO = new SceneObject("Temp");
                            temporarySO.Position = activeHandle.Position;

                            SceneObject[] originalParents = new SceneObject[activeSelection.Length];
                            for (int i = 0; i < activeSelection.Length; i++)
                            {
                                originalParents[i] = activeSelection[i].so.Parent;
                                activeSelection[i].so.LocalPosition = activeSelection[i].initialPosition;
                                activeSelection[i].so.LocalRotation = activeSelection[i].initialRotation;
                                activeSelection[i].so.LocalScale = activeSelection[i].initialScale;
                                activeSelection[i].so.Parent = temporarySO;
                            }

                            temporarySO.LocalScale += scaleHandle.Delta;

                            for (int i = 0; i < activeSelection.Length; i++)
                                activeSelection[i].so.Parent = originalParents[i];

                            temporarySO.Destroy();
                        }
                            break;
                    }

                    SceneObject[] selectedSceneObjects = new SceneObject[activeSelection.Length];
                    for (int i = 0; i < activeSelection.Length; i++)
                        selectedSceneObjects[i] = activeSelection[i].so;

                    // Make sure to update handle positions for the drawing method (otherwise they lag one frame)
                    UpdateActiveHandleTransform(selectedSceneObjects);

                    EditorApplication.SetSceneDirty();
                }
            }
            else
            {
                isDragged = false;
                activeSelection = null;
            }
        }

        /// <inheritdoc/>
        protected internal override void Draw()
        {
            if (activeHandle != null)
                activeHandle.Draw();
        }

        /// <summary>
        /// Updates active handle position/rotation based on the currently selected object(s).
        /// </summary>
        private void UpdateActiveHandleTransform(SceneObject[] selectedSceneObjects)
        {
            if (activeHandle == null)
                return;

            Quaternion rotation;
            if (EditorApplication.ActiveCoordinateMode == HandleCoordinateMode.World)
                rotation = Quaternion.Identity;
            else
                rotation = selectedSceneObjects[0].Rotation; // We don't average rotation in case of multi-selection

            Vector3 position;
            if (EditorApplication.ActivePivotMode == HandlePivotMode.Pivot)
                position = selectedSceneObjects[0].Position; // Just take pivot from the first one, no averaging
            else
            {
                List<SceneObject> flatenedHierarchy = new List<SceneObject>();
                foreach (var so in selectedSceneObjects)
                    flatenedHierarchy.AddRange(EditorUtility.FlattenHierarchy(so));

                position = EditorUtility.CalculateCenter(flatenedHierarchy.ToArray());
            }

            activeHandle.Position = position;
            activeHandle.Rotation = rotation;
        }
    }

    /** @} */
}

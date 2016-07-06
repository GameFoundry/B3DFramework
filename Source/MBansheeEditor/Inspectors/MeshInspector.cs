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
    /// Renders an inspector for the <see cref="Mesh"/> resource.
    /// </summary>
    [CustomInspector(typeof(Mesh))]
    internal class MeshInspector : Inspector
    {
        private GUIToggleField normalsField;
        private GUIToggleField tangentsField;
        private GUIToggleField skinField;
        private GUIToggleField blendShapesField;
        private GUIToggleField animationField;
        private GUIFloatField scaleField;
        private GUIToggleField cpuReadableField;
        private GUIEnumField collisionMeshTypeField;
        private GUIToggleField keyFrameReductionField;
        private GUIArrayField<AnimationSplitInfo, AnimSplitArrayRow> animSplitInfoField;
        private GUIButton reimportButton;

        private MeshImportOptions importOptions;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            if (InspectedObject != null)
            {
                importOptions = GetImportOptions();
                BuildGUI();
            }
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            MeshImportOptions newImportOptions = GetImportOptions();

            bool rebuildGUI = false;

            AnimationSplitInfo[] splitInfos = newImportOptions.AnimationClipSplits;
            if (splitInfos == null)
                rebuildGUI |= animSplitInfoField.Array != null;
            else
            {
                if (animSplitInfoField.Array == null)
                    rebuildGUI = true;
                else
                    rebuildGUI |= splitInfos.Length != animSplitInfoField.Array.GetLength(0);
            }

            if (rebuildGUI)
                BuildGUI();

            animSplitInfoField.Refresh();

            normalsField.Value = newImportOptions.ImportNormals;
            tangentsField.Value = newImportOptions.ImportTangents;
            skinField.Value = newImportOptions.ImportSkin;
            blendShapesField.Value = newImportOptions.ImportBlendShapes;
            animationField.Value = newImportOptions.ImportAnimation;
            scaleField.Value = newImportOptions.Scale;
            cpuReadableField.Value = newImportOptions.CPUReadable;
            collisionMeshTypeField.Value = (ulong)newImportOptions.CollisionMeshType;
            keyFrameReductionField.Value = newImportOptions.KeyframeReduction;

            importOptions = newImportOptions;

            return InspectableState.NotModified;
        }

        /// <summary>
        /// Recreates all the GUI elements used by this inspector.
        /// </summary>
        private void BuildGUI()
        {
            Layout.Clear();

            normalsField = new GUIToggleField(new LocEdString("Import Normals"));
            tangentsField = new GUIToggleField(new LocEdString("Import Tangents"));
            skinField = new GUIToggleField(new LocEdString("Import Skin"));
            blendShapesField = new GUIToggleField(new LocEdString("Import Blend Shapes"));
            animationField = new GUIToggleField(new LocEdString("Import Animation"));
            scaleField = new GUIFloatField(new LocEdString("Scale"));
            cpuReadableField = new GUIToggleField(new LocEdString("CPU readable"));
            collisionMeshTypeField = new GUIEnumField(typeof(CollisionMeshType), new LocEdString("Collision mesh"));
            keyFrameReductionField = new GUIToggleField(new LocEdString("Keyframe Reduction"));
            reimportButton = new GUIButton(new LocEdString("Reimport"));

            normalsField.OnChanged += x => importOptions.ImportNormals = x;
            tangentsField.OnChanged += x => importOptions.ImportTangents = x;
            skinField.OnChanged += x => importOptions.ImportSkin = x;
            blendShapesField.OnChanged += x => importOptions.ImportBlendShapes = x;
            animationField.OnChanged += x => importOptions.ImportAnimation = x;
            scaleField.OnChanged += x => importOptions.Scale = x;
            cpuReadableField.OnChanged += x => importOptions.CPUReadable = x;
            collisionMeshTypeField.OnSelectionChanged += x => importOptions.CollisionMeshType = (CollisionMeshType)x;
            keyFrameReductionField.OnChanged += x => importOptions.KeyframeReduction = x;

            reimportButton.OnClick += TriggerReimport;

            Layout.AddElement(normalsField);
            Layout.AddElement(tangentsField);
            Layout.AddElement(skinField);
            Layout.AddElement(blendShapesField);
            Layout.AddElement(animationField);
            Layout.AddElement(scaleField);
            Layout.AddElement(cpuReadableField);
            Layout.AddElement(collisionMeshTypeField);
            Layout.AddElement(keyFrameReductionField);

            animSplitInfoField = GUIArrayField<AnimationSplitInfo, AnimSplitArrayRow>.Create(
                new LocEdString("Animation splits"), importOptions.AnimationClipSplits, Layout);
            animSplitInfoField.OnChanged += x => importOptions.AnimationClipSplits = x;
            animSplitInfoField.IsExpanded = Persistent.GetBool("animSplitInfos_Expanded");
            animSplitInfoField.OnExpand += x => Persistent.SetBool("animSplitInfos_Expanded", x);

            Layout.AddSpace(10);

            GUILayout reimportButtonLayout = Layout.AddLayoutX();
            reimportButtonLayout.AddFlexibleSpace();
            reimportButtonLayout.AddElement(reimportButton); 
        }

        /// <summary>
        /// Retrieves import options for the mesh we're currently inspecting.
        /// </summary>
        /// <returns>Mesh import options object.</returns>
        private MeshImportOptions GetImportOptions()
        {
            Mesh mesh = InspectedObject as Mesh;
            MeshImportOptions output = null;

            if (mesh != null)
            {
                LibraryEntry meshEntry = ProjectLibrary.GetEntry(ProjectLibrary.GetPath(mesh));
                if (meshEntry != null && meshEntry.Type == LibraryEntryType.File)
                {
                    FileEntry meshFileEntry = (FileEntry)meshEntry;
                    output = meshFileEntry.Options as MeshImportOptions;
                }
            }

            if (output == null)
            {
                if (importOptions == null)
                    output = new MeshImportOptions();
                else
                    output = importOptions;
            }

            return output;
        }

        /// <summary>
        /// Reimports the resource according to the currently set import options.
        /// </summary>
        private void TriggerReimport()
        {
            Mesh mesh = (Mesh)InspectedObject;
            string resourcePath = ProjectLibrary.GetPath(mesh);

            ProjectLibrary.Reimport(resourcePath, importOptions, true);
        }

        /// <summary>
        /// Row element used for displaying GUI for animation clip split information.
        /// </summary>
        private class AnimSplitArrayRow : GUIListFieldRow
        {
            private GUITextField nameField;
            private GUIIntField startFrameField;
            private GUIIntField endFrameField;
            private GUIToggleField isAdditiveField;

            /// <inheritdoc/>
            protected override GUILayoutX CreateGUI(GUILayoutY layout)
            {
                GUILayoutX titleLayout = layout.AddLayoutX();
                nameField = new GUITextField(new LocEdString("Name"));
                startFrameField = new GUIIntField(new LocEdString("Start"));
                endFrameField = new GUIIntField(new LocEdString("End"));
                isAdditiveField = new GUIToggleField(new LocEdString("Is additive"));

                startFrameField.SetRange(0, int.MaxValue);
                endFrameField.SetRange(0, int.MaxValue);

                titleLayout.AddElement(nameField);
                titleLayout.AddElement(startFrameField);
                titleLayout.AddElement(endFrameField);
                titleLayout.AddElement(isAdditiveField);

                nameField.OnChanged += x =>
                {
                    AnimationSplitInfo splitInfo = GetValue<AnimationSplitInfo>();
                    splitInfo.name = x;
 
                    MarkAsModified();
                };

                nameField.OnFocusLost += ConfirmModify;
                nameField.OnConfirmed += ConfirmModify;

                startFrameField.OnChanged += x =>
                {
                    AnimationSplitInfo splitInfo = GetValue<AnimationSplitInfo>();
                    splitInfo.startFrame = x;

                    MarkAsModified();
                };

                startFrameField.OnFocusLost += ConfirmModify;
                startFrameField.OnConfirmed += ConfirmModify;

                endFrameField.OnChanged += x =>
                {
                    AnimationSplitInfo splitInfo = GetValue<AnimationSplitInfo>();
                    splitInfo.endFrame = x;

                    MarkAsModified();
                };

                endFrameField.OnFocusLost += ConfirmModify;
                endFrameField.OnConfirmed += ConfirmModify;

                isAdditiveField.OnChanged += x =>
                {
                    AnimationSplitInfo splitInfo = GetValue<AnimationSplitInfo>();
                    splitInfo.isAdditive = x;

                    MarkAsModified();
                    ConfirmModify();
                };

                return titleLayout;
            }

            /// <inheritdoc/>
            protected internal override InspectableState Refresh()
            {
                AnimationSplitInfo splitInfo = GetValue<AnimationSplitInfo>();
                nameField.Value = splitInfo.name;
                startFrameField.Value = splitInfo.startFrame;
                endFrameField.Value = splitInfo.endFrame;
                isAdditiveField.Value = splitInfo.isAdditive;

                return base.Refresh();
            }
        }
    }

    /** @} */
}

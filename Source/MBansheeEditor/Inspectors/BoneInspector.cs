//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="Bone"/> component.
    /// </summary>
    [CustomInspector(typeof(Bone))]
    internal class BoneInspector : Inspector
    {
        private GUIListBoxField boneField;
        private InspectableState modifyState;

        private string selectedBoneName;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            BuildGUI();
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            Bone bone = InspectedObject as Bone;
            if (bone == null)
                return InspectableState.NotModified;

            if (selectedBoneName != bone.Name)
            {
                string[] boneNames = GetBoneNames(bone);

                if (boneNames != null)
                {
                    for (int i = 0; i < boneNames.Length; i++)
                    {
                        if (bone.Name == boneNames[i])
                        {
                            boneField.Index = i;
                            break;
                        }
                    }
                }
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
            Layout.Clear();

            Bone bone = InspectedObject as Bone;
            if (bone == null)
                return;

            string[] boneNames = GetBoneNames(bone);
            if(boneNames == null)
                boneNames = new string[0];

            boneField = new GUIListBoxField(boneNames, false, new LocEdString("Bone"));

            Layout.AddElement(boneField);

            boneField.OnSelectionChanged += x =>
            {
                selectedBoneName = boneNames[x];
                bone.Name = selectedBoneName;
                
                MarkAsModified();
                ConfirmModify();
            };
        }

        /// <summary>
        /// Finds all available bones for the animation the provided bone is a part of.
        /// </summary>
        /// <param name="bone">Bone for which to return the parent skeleton's bones.</param>
        /// <returns>List of bones if parent skeleton is found, or null.</returns>
        private string[] GetBoneNames(Bone bone)
        {
            Animation animParent = null;

            SceneObject currentSO = bone.SceneObject;
            while (currentSO != null)
            {
                animParent = currentSO.GetComponent<Animation>();
                if (animParent != null)
                    break;

                currentSO = currentSO.Parent;
            }

            if(animParent == null)
                return null;

            Renderable renderable = animParent.SceneObject.GetComponent<Renderable>();
            if (renderable == null)
                return null;

            Mesh mesh = renderable.Mesh;
            if (mesh == null)
                return null;

            Skeleton skeleton = mesh.Skeleton;
            string[] boneNames = new string[skeleton.NumBones];
            for (int i = 0; i < boneNames.Length; i++)
                boneNames[i] = skeleton.GetBoneInfo(i).Name;

            return boneNames;
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
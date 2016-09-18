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
    /// Renders an inspector for the <see cref="Animation"/> component.
    /// </summary>
    [CustomInspector(typeof(Animation))]
    internal class AnimationInspector : Inspector
    {
        private GUIResourceField animationClipField = new GUIResourceField(typeof(AnimationClip), new LocEdString("Clip"));
        private GUIEnumField wrapModeField = new GUIEnumField(typeof(AnimWrapMode), new LocEdString("Wrap mode"));
        private GUIFloatField speedField = new GUIFloatField(new LocEdString("Speed"));
        private GUIToggleField cullingField = new GUIToggleField(new LocEdString("Culling"));
        private GUIToggleField overrideBoundsField = new GUIToggleField(new LocEdString("Override bounds"));
        private GUIVector3Field centerField = new GUIVector3Field(new LocEdString("Center"));
        private GUIVector3Field sizeField = new GUIVector3Field(new LocEdString("Size"));
        private InspectableState modifyState;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            BuildGUI();
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            Animation animation = InspectedObject as Animation;
            if (animation == null)
                return InspectableState.NotModified;

            animationClipField.Value = animation.DefaultClip;
            wrapModeField.Value = (ulong)animation.WrapMode;
            speedField.Value = animation.Speed;
            cullingField.Value = animation.Cull;
            overrideBoundsField.Value = animation.UseBounds;
            centerField.Value = animation.Bounds.Center;
            sizeField.Value = animation.Bounds.Size;

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

            Animation animation = InspectedObject as Animation;
            if (animation == null)
                return;

            animationClipField.OnChanged += x =>
            {
                AnimationClip clip = Resources.Load<AnimationClip>(x);

                animation.DefaultClip = clip;
                MarkAsModified();
                ConfirmModify();
            };

            wrapModeField.OnSelectionChanged += x =>
            {
                animation.WrapMode = (AnimWrapMode)x;
                MarkAsModified();
                ConfirmModify();
            };

            speedField.OnChanged += x => { animation.Speed = x; MarkAsModified(); };
            speedField.OnConfirmed += ConfirmModify;
            speedField.OnFocusLost += ConfirmModify;

            cullingField.OnChanged += x => { animation.Cull = x; MarkAsModified(); ConfirmModify(); };
            overrideBoundsField.OnChanged += x => { animation.UseBounds = x; MarkAsModified(); ConfirmModify(); };
            centerField.OnChanged += x =>
            {
                AABox bounds = animation.Bounds;
                Vector3 min = x - bounds.Size*0.5f;
                Vector3 max = x + bounds.Size*0.5f;

                animation.Bounds = new AABox(min, max);
                MarkAsModified();
            };
            centerField.OnConfirmed += ConfirmModify;
            centerField.OnFocusLost += ConfirmModify;

            sizeField.OnChanged += x =>
            {
                AABox bounds = animation.Bounds;
                Vector3 min = bounds.Center - x * 0.5f;
                Vector3 max = bounds.Center + x * 0.5f;

                animation.Bounds = new AABox(min, max);
                MarkAsModified();
            };
            sizeField.OnConfirmed += ConfirmModify;
            sizeField.OnFocusLost += ConfirmModify;

            Layout.AddElement(animationClipField);
            Layout.AddElement(wrapModeField);
            Layout.AddElement(speedField);
            Layout.AddElement(cullingField);
            Layout.AddElement(overrideBoundsField);

            GUILayoutX boundsLayout = Layout.AddLayoutX();
            boundsLayout.AddElement(new GUILabel(new LocEdString("Bounds"), GUIOption.FixedWidth(100)));

            GUILayoutY boundsContent = boundsLayout.AddLayoutY();
            boundsContent.AddElement(centerField);
            boundsContent.AddElement(sizeField);

            // Morph shapes
            Renderable renderable = animation.SceneObject.GetComponent<Renderable>();
            MorphShapes morphShapes = renderable?.Mesh?.MorphShapes;
            if (morphShapes != null)
            {
                GUIToggle morphShapesToggle = new GUIToggle(new LocEdString("Morph shapes"), EditorStyles.Foldout);

                Layout.AddElement(morphShapesToggle);
                GUILayoutY channelsLayout = Layout.AddLayoutY();

                morphShapesToggle.OnToggled += x =>
                {
                    channelsLayout.Active = x;
                    Persistent.SetBool("Channels_Expanded", x);
                };

                channelsLayout.Active = Persistent.GetBool("Channels_Expanded");

                MorphChannel[] channels = morphShapes.Channels;
                for (int i = 0; i < channels.Length; i++)
                {
                    GUILayoutY channelLayout = channelsLayout.AddLayoutY();

                    GUILayoutX channelTitleLayout = channelLayout.AddLayoutX();
                    GUILayoutX channelContentLayout = channelLayout.AddLayoutX();

                    string channelName = channels[i].Name;
                    GUIToggle channelNameField = new GUIToggle(channelName, EditorStyles.Foldout);

                    channelTitleLayout.AddSpace(15); // Indent
                    channelTitleLayout.AddElement(channelNameField);
                    channelTitleLayout.AddFlexibleSpace();

                    channelNameField.OnToggled += x =>
                    {
                        channelContentLayout.Active = x;

                        Persistent.SetBool(channelName + "_Expanded", x);
                    };

                    channelContentLayout.Active = Persistent.GetBool(channelName + "_Expanded");

                    MorphShape[] shapes = channels[i].Shapes;
                    for (int j = 0; j < shapes.Length; j++)
                    {
                        GUILayoutX shapeLayout = channelContentLayout.AddLayoutX();

                        GUILabel shapeNameField = new GUILabel(shapes[j].Name);

                        LocString weightString = new LocEdString("Weight: {0}");
                        weightString.SetParameter(0, shapes[j].Weight.ToString());
                        GUILabel weightField = new GUILabel(weightString);

                        shapeLayout.AddSpace(30); // Indent
                        shapeLayout.AddElement(shapeNameField);
                        shapeLayout.AddFlexibleSpace();
                        shapeLayout.AddElement(weightField);
                    }
                }
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
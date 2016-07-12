//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
            
            Layout.AddElement(animationClipField);
            Layout.AddElement(wrapModeField);
            Layout.AddElement(speedField);
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
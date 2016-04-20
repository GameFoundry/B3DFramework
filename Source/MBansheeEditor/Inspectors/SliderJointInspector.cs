//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="SliderJoint"/> component.
    /// </summary>
    [CustomInspector(typeof(SliderJoint))]
    internal class SliderJointInspector : JointInspector
    {
        private GUIToggleField enableLimitField = new GUIToggleField(new LocEdString("Enable limit"));
        private LimitLinearRangeGUI limitGUI;

        private GUILayoutX limitLayout;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            SliderJoint joint = InspectedObject as SliderJoint;

            if (joint != null)
                BuildGUI(joint);
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            SliderJoint joint = InspectedObject as SliderJoint;
            if (joint == null)
                return InspectableState.NotModified;

            Refresh(joint);

            InspectableState oldState = modifyState;
            if (modifyState.HasFlag(InspectableState.Modified))
                modifyState = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Creates GUI elements for fields specific to the slider joint.
        /// </summary>
        protected void BuildGUI(SliderJoint joint)
        {
            enableLimitField.OnChanged += x =>
            {
                joint.EnableLimit = x;
                MarkAsModified();
                ConfirmModify();

                ToggleLimitFields(x);
            };
            
            Layout.AddElement(enableLimitField);
            limitLayout = Layout.AddLayoutX();
            {
                limitLayout.AddSpace(10);

                GUILayoutY limitContentsLayout = limitLayout.AddLayoutY();
                limitGUI = new LimitLinearRangeGUI(joint.Limit, limitContentsLayout, Persistent);
                limitGUI.OnChanged += (x, y) =>
                {
                    joint.Limit = new LimitLinearRange(x, y);

                    MarkAsModified();
                };
                limitGUI.OnConfirmed += ConfirmModify;
            }
            
            ToggleLimitFields(joint.EnableLimit);

            base.BuildGUI(joint, true);
        }

        /// <summary>
        /// Updates all GUI elements from current values in the joint.
        /// </summary>
        /// <param name="joint">Joint to update the GUI from.</param>
        protected void Refresh(SliderJoint joint)
        {
            if (enableLimitField.Value != joint.EnableLimit)
            {
                enableLimitField.Value = joint.EnableLimit;
                ToggleLimitFields(joint.EnableLimit);
            }

            limitGUI.Limit = joint.Limit;
            
            base.Refresh(joint);
        }

        /// <summary>
        /// Hides or shows limit property GUI elements.
        /// </summary>
        /// <param name="enable">True to show, false to hide.</param>
        private void ToggleLimitFields(bool enable)
        {
            limitLayout.Active = enable;
        }
    }

    /** @} */
}
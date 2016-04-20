//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="HingeJoint"/> component.
    /// </summary>
    [CustomInspector(typeof(HingeJoint))]
    internal class HingeJointInspector : JointInspector
    {
        private GUIToggleField enableLimitField = new GUIToggleField(new LocEdString("Enable limit"));
        private LimitAngularRangeGUI limitGUI;

        private GUIToggleField enableDriveField = new GUIToggleField(new LocEdString("Enable drive"));
        private GUIFloatField speedField = new GUIFloatField(new LocEdString("Speed"));
        private GUIFloatField forceLimitField = new GUIFloatField(new LocEdString("Force limit"));
        private GUIFloatField gearRatioField = new GUIFloatField(new LocEdString("Gear ratio"));
        private GUIToggleField freeSpinField = new GUIToggleField(new LocEdString("Free spin"));

        private GUILayoutX limitLayout;
        private GUILayoutX driveLayout;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            HingeJoint joint = InspectedObject as HingeJoint;

            if (joint != null)
                BuildGUI(joint);
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            HingeJoint joint = InspectedObject as HingeJoint;
            if (joint == null)
                return InspectableState.NotModified;

            Refresh(joint);

            InspectableState oldState = modifyState;
            if (modifyState.HasFlag(InspectableState.Modified))
                modifyState = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Creates GUI elements for fields specific to the hinge joint.
        /// </summary>
        protected void BuildGUI(HingeJoint joint)
        {
            enableLimitField.OnChanged += x =>
            {
                joint.EnableLimit = x;
                MarkAsModified();
                ConfirmModify();

                ToggleLimitFields(x);
            };
            
            enableDriveField.OnChanged += x =>
            {
                joint.EnableDrive = x;
                MarkAsModified();
                ConfirmModify();

                ToggleDriveFields(x);
            };

            speedField.OnChanged += x =>
            {
                HingeJointDriveData driveData = joint.Drive.Data;
                driveData.speed = x;
                joint.Drive = new HingeJointDrive(driveData);

                MarkAsModified();
            };
            speedField.OnFocusLost += ConfirmModify;
            speedField.OnConfirmed += ConfirmModify;

            forceLimitField.OnChanged += x =>
            {
                HingeJointDriveData driveData = joint.Drive.Data;
                driveData.forceLimit = x;
                joint.Drive = new HingeJointDrive(driveData);

                MarkAsModified();
            };
            forceLimitField.OnFocusLost += ConfirmModify;
            forceLimitField.OnConfirmed += ConfirmModify;

            gearRatioField.OnChanged += x =>
            {
                HingeJointDriveData driveData = joint.Drive.Data;
                driveData.gearRatio = x;
                joint.Drive = new HingeJointDrive(driveData);

                MarkAsModified();
            };
            gearRatioField.OnFocusLost += ConfirmModify;
            gearRatioField.OnConfirmed += ConfirmModify;

            freeSpinField.OnChanged += x =>
            {
                HingeJointDriveData driveData = joint.Drive.Data;
                driveData.freeSpin = x;
                joint.Drive = new HingeJointDrive(driveData);

                MarkAsModified();
                ConfirmModify();
            };

            Layout.AddElement(enableLimitField);
            limitLayout = Layout.AddLayoutX();
            {
                limitLayout.AddSpace(10);

                GUILayoutY limitContentsLayout = limitLayout.AddLayoutY();
                limitGUI = new LimitAngularRangeGUI(joint.Limit, limitContentsLayout, Persistent);
                limitGUI.OnChanged += (x, y) =>
                {
                    joint.Limit = new LimitAngularRange(x, y);

                    MarkAsModified();
                };
                limitGUI.OnConfirmed += ConfirmModify;
            }

            Layout.AddElement(enableDriveField);
            driveLayout = Layout.AddLayoutX();
            {
                driveLayout.AddSpace(10);

                GUILayoutY driveContentsLayout = driveLayout.AddLayoutY();
                driveContentsLayout.AddElement(speedField);
                driveContentsLayout.AddElement(forceLimitField);
                driveContentsLayout.AddElement(gearRatioField);
                driveContentsLayout.AddElement(freeSpinField);
            }

            ToggleLimitFields(joint.EnableLimit);
            ToggleDriveFields(joint.EnableDrive);

            base.BuildGUI(joint, true);
        }

        /// <summary>
        /// Updates all GUI elements from current values in the joint.
        /// </summary>
        /// <param name="joint">Joint to update the GUI from.</param>
        protected void Refresh(HingeJoint joint)
        {
            if (enableLimitField.Value != joint.EnableLimit)
            {
                enableLimitField.Value = joint.EnableLimit;
                ToggleLimitFields(joint.EnableLimit);
            }

            limitGUI.Limit = joint.Limit;

            if (enableDriveField.Value != joint.EnableDrive)
            {
                enableDriveField.Value = joint.EnableDrive;
                ToggleDriveFields(joint.EnableDrive);
            }

            speedField.Value = joint.Drive.Speed;
            forceLimitField.Value = joint.Drive.ForceLimit;
            gearRatioField.Value = joint.Drive.GearRatio;
            freeSpinField.Value = joint.Drive.FreeSpin;

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

        /// <summary>
        /// Hides or shows drive property GUI elements.
        /// </summary>
        /// <param name="enable">True to show, false to hide.</param>
        private void ToggleDriveFields(bool enable)
        {
            driveLayout.Active = enable;
        }
    }

    /** @} */
}
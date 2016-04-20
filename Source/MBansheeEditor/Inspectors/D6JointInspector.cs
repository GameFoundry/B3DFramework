//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="D6Joint"/> component.
    /// </summary>
    [CustomInspector(typeof(D6Joint))]
    internal class D6JointInspector : JointInspector
    {
        private GUIEnumField[] motionFields = new GUIEnumField[(int) D6JointAxis.Count];

        private GUIToggle linearLimitFoldout = new GUIToggle(new LocEdString("Linear limit"), EditorStyles.Foldout);
        private LimitLinearGUI limitLinearGUI;
        private GUIToggle twistLimitFoldout = new GUIToggle(new LocEdString("Twist limit"), EditorStyles.Foldout);
        private LimitAngularRangeGUI limitTwistGUI;
        private GUIToggle swingLimitFoldout = new GUIToggle(new LocEdString("Swing limit"), EditorStyles.Foldout);
        private LimitConeRangeGUI limitSwingGUI;

        private GUIToggle driveFoldout = new GUIToggle(new LocEdString("Drive"), EditorStyles.Foldout);
        private D6JointDriveGUI[] drivesGUI = new D6JointDriveGUI[(int)D6JointDriveType.Count];
        private GUIVector3Field drivePositionField = new GUIVector3Field(new LocEdString("Drive position"));
        private GUIVector3Field driveRotationField = new GUIVector3Field(new LocEdString("Drive rotation"));
        private GUIVector3Field driveLinVelocityField = new GUIVector3Field(new LocEdString("Drive linear velocity"));
        private GUIVector3Field driveAngVelocityField = new GUIVector3Field(new LocEdString("Drive angular velocity"));

        private GUILayoutX linearLimitLayout;
        private GUILayoutX twistLimitLayout;
        private GUILayoutX swingLimitLayout;
        private GUILayoutX driveLayout;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            D6Joint joint = InspectedObject as D6Joint;

            if (joint != null)
                BuildGUI(joint);
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            D6Joint joint = InspectedObject as D6Joint;
            if (joint == null)
                return InspectableState.NotModified;

            Refresh(joint);

            InspectableState oldState = modifyState;
            if (modifyState.HasFlag(InspectableState.Modified))
                modifyState = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Creates GUI elements for fields specific to the spherical joint.
        /// </summary>
        protected void BuildGUI(D6Joint joint)
        {
            for (int i = 0; i < (int) D6JointAxis.Count; i++)
            {
                D6JointAxis axis = (D6JointAxis) i;
                string entryName = Enum.GetName(typeof (D6JointAxis), axis);

                motionFields[i] = new GUIEnumField(typeof (D6JointMotion), new LocEdString(entryName));
                motionFields[i].OnSelectionChanged += x =>
                {
                    joint.SetMotion(axis, (D6JointMotion)x);

                    MarkAsModified();
                    ConfirmModify();
                };
            }

            linearLimitFoldout.OnToggled += x =>
            {
                linearLimitLayout.Active = x;
                Persistent.SetBool("linearLimit_Expanded", x);
            };

            twistLimitFoldout.OnToggled += x =>
            {
                twistLimitLayout.Active = x;
                Persistent.SetBool("twistLimit_Expanded", x);
            };

            swingLimitFoldout.OnToggled += x =>
            {
                swingLimitLayout.Active = x;
                Persistent.SetBool("swingLimit_Expanded", x);
            };

            driveFoldout.OnToggled += x =>
            {
                driveLayout.Active = x;
                Persistent.SetBool("drive_Expanded", x);
            };

            drivePositionField.OnChanged += x => { joint.DrivePosition = x; MarkAsModified(); };
            drivePositionField.OnFocusLost += ConfirmModify;
            drivePositionField.OnConfirmed += ConfirmModify;

            driveRotationField.OnChanged += x => { joint.DriveRotation = Quaternion.FromEuler(x); MarkAsModified(); };
            driveRotationField.OnFocusLost += ConfirmModify;
            driveRotationField.OnConfirmed += ConfirmModify;

            driveLinVelocityField.OnChanged += x => { joint.DriveLinearVelocity = x; MarkAsModified(); };
            driveLinVelocityField.OnFocusLost += ConfirmModify;
            driveLinVelocityField.OnConfirmed += ConfirmModify;

            driveAngVelocityField.OnChanged += x => { joint.DriveAngularVelocity = x; MarkAsModified(); };
            driveAngVelocityField.OnFocusLost += ConfirmModify;
            driveAngVelocityField.OnConfirmed += ConfirmModify;

            for (int i = 0; i < (int) D6JointAxis.Count; i++)
                Layout.AddElement(motionFields[i]);

            Layout.AddElement(linearLimitFoldout);
            linearLimitLayout = Layout.AddLayoutX();
            {
                linearLimitLayout.AddSpace(10);
                GUILayoutY linearLimitContentsLayout = linearLimitLayout.AddLayoutY();
                limitLinearGUI = new LimitLinearGUI(joint.LimitLinear, linearLimitContentsLayout, Persistent);
                limitLinearGUI.OnChanged += (x, y) =>
                {
                    joint.LimitLinear = new LimitLinear(x, y);

                    MarkAsModified();
                };
                limitLinearGUI.OnConfirmed += ConfirmModify;
            }

            Layout.AddElement(twistLimitFoldout);
            twistLimitLayout = Layout.AddLayoutX();
            {
                twistLimitLayout.AddSpace(10);
                GUILayoutY twistLimitContentsLayout = twistLimitLayout.AddLayoutY();
                limitTwistGUI = new LimitAngularRangeGUI(joint.LimitTwist, twistLimitContentsLayout, Persistent);
                limitTwistGUI.OnChanged += (x, y) =>
                {
                    joint.LimitTwist = new LimitAngularRange(x, y);

                    MarkAsModified();
                };
                limitTwistGUI.OnConfirmed += ConfirmModify;
            }

            Layout.AddElement(swingLimitFoldout);
            swingLimitLayout = Layout.AddLayoutX();
            {
                swingLimitLayout.AddSpace(10);
                GUILayoutY swingLimitContentsLayout = swingLimitLayout.AddLayoutY();
                limitSwingGUI = new LimitConeRangeGUI(joint.LimitSwing, swingLimitContentsLayout, Persistent);
                limitSwingGUI.OnChanged += (x, y) =>
                {
                    joint.LimitSwing = new LimitConeRange(x, y);

                    MarkAsModified();
                };
                limitSwingGUI.OnConfirmed += ConfirmModify;
            }

            Layout.AddElement(driveFoldout);
            driveLayout = Layout.AddLayoutX();
            {
                driveLayout.AddSpace(10);
                GUILayoutY driveContentsLayout = driveLayout.AddLayoutY();

                for (int i = 0; i < (int) D6JointDriveType.Count; i++)
                {
                    D6JointDriveType type = (D6JointDriveType)i;

                    drivesGUI[i] = new D6JointDriveGUI(joint.GetDrive(type), driveContentsLayout);
                    drivesGUI[i].OnChanged += x => { joint.SetDrive(type, new D6JointDrive(x)); MarkAsModified(); };
                    drivesGUI[i].OnConfirmed += ConfirmModify;
                }

                driveContentsLayout.AddElement(drivePositionField);
                driveContentsLayout.AddElement(driveRotationField);
                driveContentsLayout.AddElement(driveLinVelocityField);
                driveContentsLayout.AddElement(driveAngVelocityField);
            }

            linearLimitLayout.Active = Persistent.GetBool("linearLimit_Expanded");
            twistLimitLayout.Active = Persistent.GetBool("twistLimit_Expanded");
            swingLimitLayout.Active = Persistent.GetBool("swingLimit_Expanded");
            driveLayout.Active = Persistent.GetBool("drive_Expanded");

            base.BuildGUI(joint, true);
        }

        /// <summary>
        /// Updates all GUI elements from current values in the joint.
        /// </summary>
        /// <param name="joint">Joint to update the GUI from.</param>
        protected void Refresh(D6Joint joint)
        {
            for (int i = 0; i < (int) D6JointAxis.Count; i++)
                motionFields[i].Value = (ulong)joint.GetMotion((D6JointAxis) i);

            limitLinearGUI.Limit = joint.LimitLinear;
            limitTwistGUI.Limit = joint.LimitTwist;
            limitSwingGUI.Limit = joint.LimitSwing;

            for (int i = 0; i < (int) D6JointDriveType.Count; i++)
                drivesGUI[i].Drive = joint.GetDrive((D6JointDriveType) i);

            drivePositionField.Value = joint.DrivePosition;
            driveRotationField.Value = joint.DriveRotation.ToEuler();
            driveLinVelocityField.Value = joint.DriveLinearVelocity;
            driveAngVelocityField.Value = joint.DriveAngularVelocity;

            base.Refresh(joint);
        }
    }

    /// <summary>
    /// Draws GUI elements for inspecting an <see cref="D6JointDrive"/> object.
    /// </summary>
    internal class D6JointDriveGUI
    {
        private D6JointDriveData driveData;

        private GUIFloatField stiffnessField = new GUIFloatField(new LocEdString("Stiffness"));
        private GUIFloatField dampingField = new GUIFloatField(new LocEdString("Damping"));
        private GUIFloatField forceLimitField = new GUIFloatField(new LocEdString("Force limit"));
        private GUIToggleField accelerationField = new GUIToggleField(new LocEdString("Acceleration"));

        public Action<D6JointDriveData> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current drive properties.
        /// </summary>
        public D6JointDrive Drive
        {
            set
            {
                driveData = value.Data;

                stiffnessField.Value = driveData.stiffness;
                dampingField.Value = driveData.damping;
                forceLimitField.Value = driveData.forceLimit;
                accelerationField.Value = driveData.acceleration;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the drive object.
        /// </summary>
        /// <param name="drive">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        public D6JointDriveGUI(D6JointDrive drive, GUILayout layout)
        {
            driveData = drive.Data;

            stiffnessField.OnChanged += x => { driveData.stiffness = x; MarkAsModified(); };
            stiffnessField.OnFocusLost += ConfirmModify;
            stiffnessField.OnConfirmed += ConfirmModify;

            dampingField.OnChanged += x => { driveData.damping = x; MarkAsModified(); };
            dampingField.OnFocusLost += ConfirmModify;
            dampingField.OnConfirmed += ConfirmModify;

            forceLimitField.OnChanged += x => { driveData.forceLimit = x; MarkAsModified(); };
            forceLimitField.OnFocusLost += ConfirmModify;
            forceLimitField.OnConfirmed += ConfirmModify;

            accelerationField.OnChanged += x => { driveData.acceleration = x; MarkAsModified(); ConfirmModify(); };

            layout.AddElement(stiffnessField);
            layout.AddElement(dampingField);
            layout.AddElement(forceLimitField);
            layout.AddElement(accelerationField);
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(driveData);
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        private void ConfirmModify()
        {
            if (OnConfirmed != null)
                OnConfirmed();
        }
    }

    /** @} */
}
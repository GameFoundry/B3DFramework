//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders common inspector elements for all <see cref="Joint"/> components.
    /// </summary>
    internal abstract class JointInspector : Inspector
    {
        private GUIGameObjectField targetField;
        private GUIVector3Field targetOffsetField;

        private GUIGameObjectField anchorField;
        private GUIVector3Field anchorOffsetField;

        private GUIFloatField breakForceField;
        private GUIFloatField breakTorqueField;
        private GUIToggleField collisionField;

        private bool showOffsets;
        protected InspectableState modifyState;

        /// <summary>
        /// Updates all GUI elements from current values in the joint.
        /// </summary>
        /// <param name="joint">Joint to update the GUI from.</param>
        protected virtual void Refresh(Joint joint)
        {
            targetField.Value = joint.GetRigidbody(JointBody.Target);
            anchorField.Value = joint.GetRigidbody(JointBody.Anchor);

            if (showOffsets)
            {
                targetOffsetField.Value = joint.GetPosition(JointBody.Target);
                anchorOffsetField.Value = joint.GetPosition(JointBody.Anchor);
            }

            breakForceField.Value = joint.BreakForce;
            breakTorqueField.Value = joint.BreakTorque;
            collisionField.Value = joint.EnableCollision;
        }

        /// <summary>
        /// Creates GUI elements for fields common to all joints.
        /// </summary>
        protected virtual void BuildGUI(Joint joint, bool showOffsets)
        {
            this.showOffsets = showOffsets;

            targetField = new GUIGameObjectField(typeof(Rigidbody), new LocEdString("Target"));
            anchorField = new GUIGameObjectField(typeof(Rigidbody), new LocEdString("Anchor"));
            
            if (showOffsets)
            {
                targetOffsetField = new GUIVector3Field(new LocEdString("Target offset"));
                anchorOffsetField = new GUIVector3Field(new LocEdString("Anchor offset"));
            }

            breakForceField = new GUIFloatField(new LocEdString("Break force"));
            breakTorqueField = new GUIFloatField(new LocEdString("Break torque"));
            collisionField = new GUIToggleField(new LocEdString("Enable collision"));

            targetField.OnChanged += x => { joint.SetRigidbody(JointBody.Target, (Rigidbody)x); MarkAsModified(); ConfirmModify(); };
            anchorField.OnChanged += x => { joint.SetRigidbody(JointBody.Anchor, (Rigidbody)x); MarkAsModified(); ConfirmModify(); };

            if(showOffsets)
            { 
                targetOffsetField.OnChanged += x => { joint.SetPosition(JointBody.Target, x); MarkAsModified(); };
                targetOffsetField.OnFocusLost += ConfirmModify;
                targetOffsetField.OnConfirmed += ConfirmModify;

                anchorOffsetField.OnChanged += x => { joint.SetPosition(JointBody.Anchor, x); MarkAsModified(); };
                anchorOffsetField.OnFocusLost += ConfirmModify;
                anchorOffsetField.OnConfirmed += ConfirmModify;
            }

            breakForceField.OnChanged += x => { joint.BreakForce = x; MarkAsModified(); };
            breakForceField.OnFocusLost += ConfirmModify;
            breakForceField.OnConfirmed += ConfirmModify;

            breakTorqueField.OnChanged += x => { joint.BreakTorque = x; MarkAsModified(); };
            breakTorqueField.OnFocusLost += ConfirmModify;
            breakTorqueField.OnConfirmed += ConfirmModify;

            collisionField.OnChanged += x => { joint.EnableCollision = x; MarkAsModified(); ConfirmModify(); };
            
            Layout.AddElement(targetField);
            if(showOffsets)
                Layout.AddElement(targetOffsetField);
            Layout.AddElement(anchorField);
            if(showOffsets)
                Layout.AddElement(anchorOffsetField);
            Layout.AddElement(breakForceField);
            Layout.AddElement(breakTorqueField);
            Layout.AddElement(collisionField);
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

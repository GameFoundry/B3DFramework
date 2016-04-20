//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="Rigidbody"/> component.
    /// </summary>
    [CustomInspector(typeof(Rigidbody))]
    internal class RigidbodyInspector : Inspector
    {
        private GUIToggleField kinematicField = new GUIToggleField(new LocEdString("Kinematic"));
        private GUIToggleField applyGravityField = new GUIToggleField(new LocEdString("Apply gravity"));
        private GUIToggleField automaticMassField = new GUIToggleField(new LocEdString("Calculate mass"));
        private GUIFloatField massField = new GUIFloatField(new LocEdString("Mass"));
        private GUIFloatField linearDragField = new GUIFloatField(new LocEdString("Linear drag"));
        private GUIFloatField angularDragField = new GUIFloatField(new LocEdString("Angular drag"));
        private GUIEnumField reportModeField = new GUIEnumField(typeof (CollisionReportMode),
            new LocEdString("Collision report mode"));
        private GUIToggleField ccdField = new GUIToggleField(new LocEdString("Continous"));

        private InspectableState modifyState;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            BuildGUI();
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            Rigidbody body = InspectedObject as Rigidbody;
            if (body == null)
                return InspectableState.NotModified;

            kinematicField.Value = body.Kinematic;
            applyGravityField.Value = body.UseGravity;
            automaticMassField.Value = (body.Flags & RigidbodyFlag.AutoMass) != 0;
            massField.Value = body.Mass;
            linearDragField.Value = body.Drag;
            angularDragField.Value = body.AngularDrag;
            reportModeField.Value = (ulong) body.CollisionReportMode;
            ccdField.Value = (body.Flags & RigidbodyFlag.CCD) != 0;

            massField.Active = (body.Flags & RigidbodyFlag.AutoMass) == 0;
            
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
            Rigidbody body = InspectedObject as Rigidbody;
            if (body == null)
                return;

            kinematicField.OnChanged += x => { body.Kinematic = x; MarkAsModified(); ConfirmModify(); };
            applyGravityField.OnChanged += x => { body.UseGravity = x; MarkAsModified(); ConfirmModify(); };
            automaticMassField.OnChanged += x =>
            {
                if (x)
                    body.Flags |= RigidbodyFlag.AutoMass;
                else
                    body.Flags &= ~RigidbodyFlag.AutoMass;

                massField.Active = !x;

                MarkAsModified();
                ConfirmModify();
            };

            massField.OnChanged += x => { body.Mass = x; MarkAsModified(); };
            massField.OnConfirmed += ConfirmModify;
            massField.OnFocusLost += ConfirmModify;

            linearDragField.OnChanged += x => { body.Drag = x; MarkAsModified(); };
            linearDragField.OnConfirmed += ConfirmModify;
            linearDragField.OnFocusLost += ConfirmModify;

            angularDragField.OnChanged += x => { body.AngularDrag = x; MarkAsModified(); };
            angularDragField.OnConfirmed += ConfirmModify;
            angularDragField.OnFocusLost += ConfirmModify;

            reportModeField.OnSelectionChanged += x =>
            {
                body.CollisionReportMode = (CollisionReportMode) x;

                MarkAsModified();
                ConfirmModify();
            };

            ccdField.OnChanged += x =>
            {
                if (x)
                    body.Flags |= RigidbodyFlag.CCD;
                else
                    body.Flags &= ~RigidbodyFlag.CCD;

                MarkAsModified();
                ConfirmModify();
            };

            massField.Active = (body.Flags & RigidbodyFlag.AutoMass) == 0;

            Layout.AddElement(kinematicField);
            Layout.AddElement(applyGravityField);
            Layout.AddElement(automaticMassField);
            Layout.AddElement(massField);
            Layout.AddElement(linearDragField);
            Layout.AddElement(angularDragField);
            Layout.AddElement(reportModeField);
            Layout.AddElement(ccdField);
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
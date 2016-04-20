//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="FixedJoint"/> component.
    /// </summary>
    [CustomInspector(typeof(FixedJoint))]
    internal class FixedJointInspector : JointInspector
    {
        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            FixedJoint joint = InspectedObject as FixedJoint;

            if (joint != null)
                BuildGUI(joint, false);
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            FixedJoint joint = InspectedObject as FixedJoint;
            if (joint == null)
                return InspectableState.NotModified;

            Refresh(joint);

            InspectableState oldState = modifyState;
            if (modifyState.HasFlag(InspectableState.Modified))
                modifyState = InspectableState.NotModified;

            return oldState;
        }
    }

    /** @} */
}
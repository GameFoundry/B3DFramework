//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
namespace BansheeEngine
{
    /** @addtogroup Physics
     *  @{
     */

    /// <summary>
    /// Physics joint that will maintain a fixed distance and orientation between its two attached bodies.
    /// </summary>
    public sealed class FixedJoint : Joint
    {
        /// <inheritdoc/>
        protected override void GetLocalTransform(JointBody body, out Vector3 position, out Quaternion rotation)
        {
            position = commonData.positions[(int)body];
            rotation = commonData.rotations[(int)body];

            Rigidbody rigidbody = commonData.bodies[(int)body];
            if (rigidbody == null) // Get world space transform if not relative to any body
            {
                Quaternion worldRot = SceneObject.Rotation;

                rotation = worldRot * rotation;
                position = worldRot.Rotate(position) + SceneObject.Position;
            }
            else // Get transform of the object relative to the joint
            {
                // Find world space transform
                Quaternion worldRot = rigidbody.SceneObject.Rotation;

                rotation = worldRot * rotation;
                position = worldRot.Rotate(position) + rigidbody.SceneObject.Position;

                // Get transform of the joint local to the object
                Quaternion invRotation = rotation.Inverse;

                position = invRotation.Rotate(SceneObject.Position - position);
                rotation = invRotation * SceneObject.Rotation;
            }
        }

        /// <inheritdoc/>
        internal override NativeJoint CreateNative()
        {
            NativeFixedJoint joint = new NativeFixedJoint(commonData.@internal);

            return joint;
        }
    }

    /** @} */
}
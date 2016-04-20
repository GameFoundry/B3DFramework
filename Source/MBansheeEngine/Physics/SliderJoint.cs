//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
namespace BansheeEngine
{
    /** @addtogroup Physics
     *  @{
     */

    /// <summary>
    /// Joint that removes all but a single translational degree of freedom. Bodies are allowed to move along a single axis.
    /// </summary>
    public sealed class SliderJoint : Joint
    {
        [SerializeField]
        private SerializableData data = new SerializableData();

        /// <summary>
        /// Determines the limit that constrains the movement of the joint to a specific minimum and maximum distance. You
        /// must enable <see cref="EnableLimit"/> for this to be enforced.
        /// </summary>
        public LimitLinearRange Limit
        {
            get { return data.@internal.limit; }
            set
            {
                if (data.@internal.limit == value)
                    return;

                data.@internal.limit = value;

                if (Native != null)
                    Native.Limit = value;
            }
        }

        /// <summary>
        /// Enables or disables the limit that clamps the movement of the joint.
        /// </summary>
        public bool EnableLimit
        {
            get { return data.@internal.enableLimit; }
            set
            {
                if (data.@internal.enableLimit == value)
                    return;

                data.@internal.enableLimit = value;

                if (Native != null)
                    Native.EnableLimit = value;
            }
        }

        /// <summary>
        /// Returns the native joint wrapped by this component.
        /// </summary>
        private NativeSliderJoint Native
        {
            get { return (NativeSliderJoint)native; }
        }

        /// <inheritdoc/>
        internal override NativeJoint CreateNative()
        {
            NativeSliderJoint joint = new NativeSliderJoint(commonData.@internal, data.@internal);

            return joint;
        }

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
            else
            {
                // Use only the offset for positioning, but for rotation use both the offset and target SO rotation.
                // (Needed because we need to rotate the joint SO in order to orient the slider direction, so we need an
                // additional transform that allows us to orient the object)
                position = rotation.Rotate(position);
                rotation = (rigidbody.SceneObject.Rotation*rotation).Inverse*SceneObject.Rotation;
            }
        }

        /// <summary>
        /// Holds all data the joint component needs to persist through serialization.
        /// </summary>
        [SerializeObject]
        internal new class SerializableData
        {
            public ScriptSliderJointData @internal;

            public SerializableData()
            {
                @internal.limit = new LimitLinearRange();
                @internal.enableLimit = false;
            }
        }
    }

    /** @} */
}

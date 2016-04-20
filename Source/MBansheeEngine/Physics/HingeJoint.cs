//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Physics
     *  @{
     */

    /// <summary>
    /// Hinge joint removes all but a single rotation degree of freedom from its two attached bodies (for example a door
    /// hinge).
    /// </summary>
    public sealed class HingeJoint : Joint
    {
        /// <summary>
        /// Flags to control hinge joint behaviour.
        /// </summary>
        [Flags]
        internal enum Flag // Note: Must match C++ enum HingeJoint::Flag
        {
            Limit = 0x01,
            Drive = 0x02,
        }

        [SerializeField]
        private SerializableData data = new SerializableData();

        /// <summary>
        /// Returns the current angle between the two attached bodes.
        /// </summary>
        public Radian Angle
        {
            get
            {
                if (Native != null)
                    return Native.Angle;

                return new Radian(0.0f);
            }
        }

        /// <summary>
        /// Returns the current angular speed of the joint.
        /// </summary>
        public float Speed
        {
            get
            {
                if (Native != null)
                    return Native.Speed;

                return 0.0f;
            }
        }

        /// <summary>
        /// Determines the limit of the joint. Limit constrains the motion to the specified angle range. You must enable
        /// <see cref="EnableLimit"/> for this to be enforced.
        /// </summary>
        public LimitAngularRange Limit
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
        /// Determines the drive properties of the joint. It drives the joint's angular velocity towards a particular value.
        /// You must enable <see cref="EnableDrive"/> for this to be applied.
        /// </summary>
        public HingeJointDrive Drive
        {
            get { return data.@internal.drive; }
            set
            {
                if (data.@internal.drive == value)
                    return;

                data.@internal.drive = value;

                if (Native != null)
                    Native.Drive = value;
            }
        }

        /// <summary>
        /// Enables or disables a limit that contrains the joint's motion to a specified angle range.
        /// </summary>
        public bool EnableLimit
        {
            get { return (data.@internal.flags & Flag.Limit) != 0; }
            set
            {
                if (!SetFlag(Flag.Limit, value))
                    return;

                if (Native != null)
                    Native.EnableLimit = value;
            }
        }

        /// <summary>
        /// Enables or disables a drive that drives the joint's angular velocity towards a particular value.
        /// </summary>
        public bool EnableDrive
        {
            get { return (data.@internal.flags & Flag.Drive) != 0; }
            set
            {
                if (!SetFlag(Flag.Drive, value))
                    return;

                if (Native != null)
                    Native.EnableDrive = value;
            }
        }

        /// <summary>
        /// Returns the native joint wrapped by this component.
        /// </summary>
        private NativeHingeJoint Native
        {
            get { return (NativeHingeJoint)native; }
        }

        /// <summary>
        /// Toggles a specific distance joint flag on or off.
        /// </summary>
        /// <param name="flag">Flag to toggle.</param>
        /// <param name="enabled">Should the flag be turned on or off.</param>
        /// <returns>True if the new newly set flag state was different from the previous one.</returns>
        private bool SetFlag(Flag flag, bool enabled)
        {
            Flag newFlags = data.@internal.flags;

            if (enabled)
                newFlags |= flag;
            else
                newFlags &= ~flag;

            if (newFlags == data.@internal.flags)
                return false;

            data.@internal.flags = newFlags;
            return true;
        }

        /// <inheritdoc/>
        internal override NativeJoint CreateNative()
        {
            NativeHingeJoint joint = new NativeHingeJoint(commonData.@internal, data.@internal);

            return joint;
        }

        /// <summary>
        /// Holds all data the joint component needs to persist through serialization.
        /// </summary>
        [SerializeObject]
        internal new class SerializableData
        {
            public ScriptHingeJointData @internal;

            public SerializableData()
            {
                @internal.limit = new LimitAngularRange();
                @internal.drive = new HingeJointDrive();
                @internal.flags = 0;
            }
        }
    }

    /** @} */
}

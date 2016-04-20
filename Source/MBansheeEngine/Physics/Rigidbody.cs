//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;

namespace BansheeEngine
{
    /** @addtogroup Physics
     *  @{
     */

    /// <summary>
    /// Rigidbody is a dynamic physics object that can be moved using forces (or directly). It will interact with other
    /// static and dynamic physics objects in the scene accordingly (it will push other non-kinematic rigidbodies,
    /// and collide with static objects).
    /// 
    /// The shape and mass of a rigidbody is governed by its colliders. You must attach at least one collider for the
    /// rigidbody to be valid. Colliders that are on the same scene object as the rigidbody, or on child scene objects
    /// are automatically considered as part of the rigidbody.
    /// </summary>
    public sealed class Rigidbody : Component
    {
        internal NativeRigidbody native;
        private List<Collider> children = new List<Collider>();
        private Joint parentJoint;

        [SerializeField]
        internal SerializableData serializableData = new SerializableData();

        /// <summary>
        /// Triggered when some object starts interacting with one of the child colliders. Only triggered if proper
        /// collision report mode is turned on.
        /// </summary>
        public event Action<CollisionData> OnCollisionBegin;

        /// <summary>
        /// Triggered for every frame that an object remains interacting with one of the child colliders. Only triggered if
        /// proper collision report mode is turned on.
        /// </summary>
        public event Action<CollisionData> OnCollisionStay;

        /// <summary>
        /// Triggered when some object stops interacting with one of the child colliders. Only triggered if proper collision
        /// report mode is turned on.
        /// </summary>
        public event Action<CollisionData> OnCollisionEnd;

        /// <summary>
        /// Determines the mass of the object and all of its collider shapes. Only relevant if RigidbodyFlag.AutoMass or 
        /// RigidbodyFlag.AutoTensors is turned off. Value of zero means the object is immovable (but can be rotated).
        /// </summary>
        public float Mass
        {
            get { return serializableData.mass; }
            set
            {
                serializableData.mass = value;

                if (native != null)
                    native.Mass = value;
            }
        }

        /// <summary>
        /// Determines if the body is kinematic. Kinematic body will not move in response to external forces (for example
        /// gravity, or another object pushing it), essentially behaving like collider. Unlike a collider though, you can
        /// still move the object and have other dynamic objects respond correctly (that is, it will push other objects).
        /// </summary>
        public bool Kinematic
        {
            get { return serializableData.isKinematic; }
            set
            {
                if (serializableData.isKinematic == value)
                    return;

                serializableData.isKinematic = value;

                if (native != null)
                {
                    native.Kinematic = value;

                    ClearColliders();
                    UpdateColliders();
                }
            }
        }

        /// <summary>
        /// Determines if the body is sleeping. Objects that aren't moved/rotated for a while are put to sleep to reduce 
        /// load on the physics system. You may also manually force objects to sleep or wake up.
        /// </summary>
        public bool Sleeping
        {
            get
            {
                if (native != null)
                    return native.Sleeping;

                return true;
            }
            set
            {
                if (native != null)
                    native.Sleeping = value;
            }
        }

        /// <summary>
        /// Threshold of force and torque under which the object will be considered to be put to sleep.
        /// </summary>
        public float SleepThreshold
        {
            get { return serializableData.sleepThreshold; }
            set
            {
                serializableData.sleepThreshold = value;

                if (native != null)
                    native.SleepThreshold = value;
            }
        }

        /// <summary>
        /// Determines whether or not the rigidbody will have the global gravity force applied to it.
        /// </summary>
        public bool UseGravity
        {
            get { return serializableData.useGravity; }
            set
            {
                serializableData.useGravity = value;

                if (native != null)
                    native.UseGravity = value;
            }
        }

        /// <summary>
        /// Determines current linear velocity of the body.
        /// </summary>
        public Vector3 Velocity
        {
            get
            {
                if (native != null)
                    return native.Velocity;

                return Vector3.Zero;
            }
            set
            {
                if (native != null)
                    native.Velocity = value;
            }
        }

        /// <summary>
        /// Determines current angular velocity of the body.
        /// </summary>
        public Vector3 AngularVelocity
        {
            get
            {
                if (native != null)
                    return native.AngularVelocity;

                return Vector3.Zero;
            }
            set
            {
                if (native != null)
                    native.AngularVelocity = value;
            }
        }

        /// <summary>
        /// Determines linear drag of the body. Higher drag values means the object resists linear movement more.
        /// </summary>
        public float Drag
        {
            get { return serializableData.linearDrag; }
            set
            {
                serializableData.linearDrag = value;

                if (native != null)
                    native.Drag = value;
            }
        }

        /// <summary>
        /// Determines angular drag of the body. Higher drag values means the object resists angular movement more.
        /// </summary>
        public float AngularDrag
        {
            get { return serializableData.angularDrag; }
            set
            {
                serializableData.angularDrag = value;

                if (native != null)
                    native.AngularDrag = value;
            }
        }

        /// <summary>
        /// Sets the inertia tensor in local mass space. Inertia tensor determines how difficult is to rotate the object. 
        /// Values of zero in the inertia tensor mean the object will be unable to rotate around a specific axis. Changing
        /// this value is only relevant if RigidbodyFlag.AutoTensors is turned off.
        /// </summary>
        public Vector3 InertiaTensor
        {
            get { return serializableData.inertiaTensor; }
            set
            {
                serializableData.inertiaTensor = value;

                if (native != null)
                    native.InertiaTensor = value;
            }
        }

        /// <summary>
        /// Determines position of the center of the mass. Changing this value is only relevant if RigidbodyFlag.AutoTensors
        /// is turned off.
        /// </summary>
        public Vector3 CenterOfMassPosition
        {
            get { return serializableData.centerMassPosition; }
            set
            {
                serializableData.centerMassPosition = value;

                if (native != null)
                    native.CenterOfMassPosition = value;
            }
        }

        /// <summary>
        /// Determines rotation of the inertia tensor (rotation of the center of mass frame). Changing this value is only 
        /// relevant if RigidbodyFlag.AutoTensors is turned off.
        /// </summary>
        public Quaternion CenterOfMassRotation
        {
            get { return serializableData.centerMassRotation; }
            set
            {
                serializableData.centerMassRotation = value;

                if (native != null)
                    native.CenterOfMassRotation = value;
            }
        }

        /// <summary>
        /// Determines maximum angular velocity of the rigidbody. Velocity will be clamped to this value.
        /// </summary>
        public float MaxAngularVelocity
        {
            get { return serializableData.maxAngularVelocity; }
            set
            {
                serializableData.maxAngularVelocity = value;

                if (native != null)
                    native.MaxAngularVelocity = value;
            }
        }

        /// <summary>
        /// Determines number of iterations to use when solving for position. Higher values can improve precision and 
        /// numerical stability of the simulation.
        /// </summary>
        public int PositionSolverCount
        {
            get { return serializableData.positionSolverCount; }
            set
            {
                serializableData.positionSolverCount = value;

                if (native != null)
                    serializableData.positionSolverCount = value;
            }
        }

        /// <summary>
        /// Determines number of iterations to use when solving for velocity. Higher values can improve precision and 
        /// numerical stability of the simulation.
        /// </summary>
        public int VelocitySolverCount
        {
            get { return serializableData.velocitySolverCount; }
            set
            {
                serializableData.velocitySolverCount = value;

                if (native != null)
                    serializableData.velocitySolverCount = value;
            }
        }

        /// <summary>
        /// Determines which (if any) collision events are reported.
        /// </summary>
        public CollisionReportMode CollisionReportMode
        {
            get { return serializableData.collisionReportMode; }
            set
            {
                if (serializableData.collisionReportMode == value)
                    return;

                serializableData.collisionReportMode = value;

                foreach (var entry in children)
                    entry.UpdateCollisionReportMode();
            }
        }

        /// <summary>
        /// Various flags that control the behaviour of the rigidbody.
        /// </summary>
        public RigidbodyFlag Flags
        {
            get { return serializableData.flags; }
            set
            {
                if (serializableData.flags == value)
                    return;

                serializableData.flags = value;

                if (native != null)
                {
                    native.Flags = value;
                    native.UpdateMassDistribution();
                }
            }
        }

        /// <summary>
        /// Moves the rigidbody to a specific position. This method will ensure physically correct movement, meaning the 
        /// body will collide with other objects along the way.
        /// </summary>
        /// <param name="position">New position for the body, in world space.</param>
        public void Move(Vector3 position)
        {
            if (native != null)
                native.Move(position);

            NotifyFlags = TransformChangedFlags.None;
            SceneObject.Position = position;
            NotifyFlags = TransformChangedFlags.Transform | TransformChangedFlags.Parent;
        }

        /// <summary>
        /// Rotates the rigidbody. This method will ensure physically correct rotation, meaning the body will collide with
        /// other objects along the way.
        /// </summary>
        /// <param name="rotation">New orientation of the body, in world space.</param>
        public void Rotate(Quaternion rotation)
        {
            if (native != null)
                native.Rotate(rotation);

            NotifyFlags = TransformChangedFlags.None;
            SceneObject.Rotation = rotation;
            NotifyFlags = TransformChangedFlags.Transform | TransformChangedFlags.Parent;
        }

        /// <summary>
        /// Applies a force to the center of the mass of the rigidbody. This will produce linear momentum.
        /// </summary>
        /// <param name="force">Force to apply.</param>
        /// <param name="mode">Determines what type of force was applied.</param>
        public void AddForce(Vector3 force, ForceMode mode = ForceMode.Force)
        {
            if (native != null)
                native.AddForce(force, mode);
        }

        /// <summary>
        /// Applies a torque to the rigidbody. This will produce angular momentum.
        /// </summary>
        /// <param name="torque">Torque to apply.</param>
        /// <param name="mode">Determines what type of torque was applied.</param>
        public void AddTorque(Vector3 torque, ForceMode mode = ForceMode.Force)
        {
            if (native != null)
                native.AddTorque(torque, mode);
        }

        /// <summary>
        /// Applies a force to a specific point on the rigidbody. This will in most cases produce both linear and angular
        /// momentum.
        /// </summary>
        /// <param name="force">Force to apply.</param>
        /// <param name="position">World space point to apply the force at.</param>
        /// <param name="mode">Determines what type of force was applied.</param>
        public void AddForceAtPoint(Vector3 force, Vector3 position, PointForceMode mode = PointForceMode.Force)
        {
            if (native != null)
                native.AddForceAtPoint(force, position, mode);
        }

        /// <summary>
        /// Returns the total (linear + angular) velocity at a specific point. 
        /// </summary>
        /// <param name="position">Point in world space.</param>
        /// <returns>Total velocity of the point.</returns>
        public Vector3 GetVelocityAtPoint(Vector3 position)
        {
            if (native != null)
                return native.GetVelocityAtPoint(position);

            return position;
        }

        /// <summary>
        /// Triggered when one of the child colliders begins touching another object.
        /// </summary>
        /// <param name="data">Data about the collision.</param>
        internal void DoOnCollisionBegin(CollisionData data)
        {
            if (OnCollisionBegin != null)
                OnCollisionBegin(data);
        }

        /// <summary>
        /// Triggered when one of the child colliders ends touching another object.
        /// </summary>
        /// <param name="data">Data about the collision.</param>
        internal void DoOnCollisionStay(CollisionData data)
        {
            if (OnCollisionStay != null)
                OnCollisionStay(data);
        }

        /// <summary>
        /// Triggered when one of the child colliders ends touching another object.
        /// </summary>
        /// <param name="data">Data about the collision.</param>
        internal void DoOnCollisionEnd(CollisionData data)
        {
            if (OnCollisionEnd != null)
                OnCollisionEnd(data);
        }

        /// <summary>
        /// Sets that joint that this rigidbody is attached to. Allows the rigidbody to notify the joint when it moves.
        /// </summary>
        /// <param name="joint">Joint the rigidbody is attached to, or null if none.</param>
        internal void SetJoint(Joint joint)
        {
            parentJoint = joint;
        }

        /// <summary>
        /// Recalculates rigidbody's mass, inertia tensors and center of mass depending on the currently set child 
        /// colliders. This should be called whenever relevant child collider properties change(like mass or shape).
        /// 
        /// If automatic tensor calculation is turned off then this will do nothing. If automatic mass calculation is turned
        /// off then this will use the mass set directly on the body using <see cref="Mass"/>.
        /// </summary>
        internal void UpdateMassDistribution()
        {
            if (native != null)
                native.UpdateMassDistribution();
        }

        /// <summary>
        /// Unregisters all child colliders from the Rigidbody.
        /// </summary>
        internal void ClearColliders()
        {
            foreach (var collider in children)
                collider.SetRigidbody(null, true);

            children.Clear();

            if (native != null)
                native.RemoveColliders();
        }

        /// <summary>
        /// Registers a new collider with the Rigidbody. This collider will then be used to calculate Rigidbody's geometry
        /// used for collisions, and optionally (depending on set flags) total mass, inertia tensors and center of mass.
        /// </summary>
        /// <param name="collider">Collider to register.</param>
        internal void AddCollider(Collider collider)
	    {
		    if (native == null)
			    return;

            children.Add(collider);
            native.AddCollider(collider);
	    }

        /// <summary>
        /// Unregisters the collider from the Rigidbody.
        /// </summary>
        /// <param name="collider">Collider to unregister.</param>
        internal void RemoveCollider(Collider collider)
        {
            if (native == null)
                return;

            if (children.Exists(x => x == collider))
            {
                native.RemoveCollider(collider);
                children.Remove(collider);
            }
        }

        private void OnInitialize()
        {
            NotifyFlags = TransformChangedFlags.Transform | TransformChangedFlags.Parent;
        }

        private void OnEnable()
        {
            RestoreNative();
        }

        private void OnDisable()
        {
            DestroyNative();
        }

        private void OnDestroy()
        {
            DestroyNative();
        }

        private void OnTransformChanged(TransformChangedFlags flags)
        {
            if (!SceneObject.Active)
                return;

            if ((flags & TransformChangedFlags.Parent) != 0)
            {
                ClearColliders();
                UpdateColliders();

                if ((serializableData.flags & RigidbodyFlag.AutoTensors) != 0)
                    native.UpdateMassDistribution();

#if DEBUG
			CheckForNestedRigibody();
#endif
            }

            native.Position = SceneObject.Position;
            native.Rotation = SceneObject.Rotation;

            if (parentJoint != null)
                parentJoint.NotifyRigidbodyMoved(this);
        }

        /// <summary>
        /// Searches child scene objects for Collider components and attaches them to the rigidbody. Make sure to call
        /// <see cref="ClearColliders"/> if you need to clear old colliders first.
        /// </summary>
        private void UpdateColliders()
        {
            Stack<SceneObject> todo = new Stack<SceneObject>();
            todo.Push(SceneObject);

            while (todo.Count > 0)
            {
                SceneObject currentSO = todo.Pop();

                if (currentSO.GetComponent<Collider>() != null)
                {
                    Collider[] colliders = currentSO.GetComponents<Collider>();
                    foreach (var entry in colliders)
                    {
                        if (!entry.IsValidParent(this))
                            continue;

                        if (entry.native == null)
                            continue; 

                        entry.SetRigidbody(this, true);
                        entry.native.Rigidbody = native;

                        children.Add(entry);
                        native.AddCollider(entry);
                    }
                }

                int childCount = currentSO.GetNumChildren();
                for (int i = 0; i < childCount; i++)
                {
                    SceneObject child = currentSO.GetChild(i);

                    if (child.GetComponent<Rigidbody>() != null)
                        continue;

                    todo.Push(child);
                }
            }
        }

        /// <summary>
        /// Checks if the rigidbody is nested under another rigidbody, and throws out a warning if so.
        /// </summary>
        private void CheckForNestedRigibody()
        {
            SceneObject currentSO = SceneObject.Parent;

            while (currentSO != null)
            {
                if (currentSO.GetComponent<Rigidbody>() != null)
                {
                    Debug.LogWarning("Nested Rigidbodies detected. This will result in inconsistent transformations. " +
                        "To parent one Rigidbody to another move its colliders to the new parent, but remove the " +
                        "Rigidbody component.");
                    return;
                }

                currentSO = currentSO.Parent;
            }
        }

        /// <summary>
        /// Destroys the internal rigidbody representation.
        /// </summary>
        private void DestroyNative()
        {
            ClearColliders();

            if (native != null)
            {
                native.Destroy();
                native = null;
            }
        }

        /// <summary>
        /// Restores internal rigidbody representation and assigns it the properties stored by the component.
        /// </summary>
        private void RestoreNative()
        {
            native = new NativeRigidbody(SceneObject);
            native.Component = this;

            UpdateColliders();

#if DEBUG
		    CheckForNestedRigibody();
#endif

            native.Position = SceneObject.Position;
            native.Rotation = SceneObject.Rotation;

            // Note: Merge into one call to avoid many virtual function calls
            native.PositionSolverCount = serializableData.positionSolverCount;
            native.VelocitySolverCount = serializableData.velocitySolverCount;
            native.MaxAngularVelocity = serializableData.maxAngularVelocity;
            native.Drag = serializableData.linearDrag;;
            native.AngularDrag = serializableData.angularDrag;
            native.SleepThreshold = serializableData.sleepThreshold;
            native.UseGravity = serializableData.useGravity;
            native.Kinematic = serializableData.isKinematic;
            native.Flags = serializableData.flags;

            if ((serializableData.flags & RigidbodyFlag.AutoTensors) == 0)
            {
                native.CenterOfMassPosition = serializableData.centerMassPosition;
                native.CenterOfMassRotation = serializableData.centerMassRotation;
                native.InertiaTensor = serializableData.inertiaTensor;
                native.Mass = serializableData.mass;
            }
            else
            {
                if ((serializableData.flags & RigidbodyFlag.AutoMass) == 0)
                    native.Mass = serializableData.mass;

                native.UpdateMassDistribution();
            }
        }

        /// <summary>
        /// Holds all data the rigidbody component needs to persist through serialization.
        /// </summary>
        [SerializeObject]
        internal class SerializableData
        {
            public int positionSolverCount = 4;
            public int velocitySolverCount = 1;
            public RigidbodyFlag flags = RigidbodyFlag.AutoTensors | RigidbodyFlag.AutoMass;
            public CollisionReportMode collisionReportMode = CollisionReportMode.None;
            public Vector3 centerMassPosition = Vector3.Zero;
            public Quaternion centerMassRotation = Quaternion.Identity;
            public Vector3 inertiaTensor = Vector3.Zero;
            public float mass = 0.0f;
            public float maxAngularVelocity = 1.0f;
            public float linearDrag = 0.0f;
            public float angularDrag = 0.0f;
            public float sleepThreshold = 0.0f;
            public bool useGravity = true;
            public bool isKinematic = false;
        }
    }

    /// <summary>
    /// Type of force or torque that can be applied to a rigidbody.
    /// </summary>
    public enum ForceMode
    {
        /// <summary>
        /// Value applied is a force.
        /// </summary>
        Force,
        /// <summary>
        /// Value applied is an impulse (a direct change in its linear or angular momentum).
        /// </summary>
		Impulse,
        /// <summary>
        /// Value applied is velocity.
        /// </summary>
		Velocity,
        /// <summary>
        /// Value applied is accelearation.
        /// </summary>
		Acceleration
    }

    /// <summary>
    /// Type of force that can be applied to a rigidbody at an arbitrary point.
    /// </summary>
    public enum PointForceMode
    {
        /// <summary>
        /// Value applied is a force.
        /// </summary>
        Force,
        /// <summary>
        /// Value applied is an impulse (a direct change in its linear or angular momentum).
        /// </summary>
		Impulse,
    }

    /// <summary>
    /// Flags that control options of a Rigidbody object.
    /// </summary>
    [Flags]
    public enum RigidbodyFlag
    {
        /// <summary>
        /// No options. 
        /// </summary>
        None = 0x00,
        /// <summary>
        /// Automatically calculate center of mass transform and inertia tensors from child shapes (colliders)
        /// </summary>
        AutoTensors = 0x01,
        /// <summary>
        /// Calculate mass distribution from child shapes (colliders). Only relevant when auto-tensors is on.
        /// </summary>
        AutoMass = 0x02,
        /// <summary>
        /// Enables continous collision detection. This can prevent fast moving bodies from tunneling through each other.
        /// This must also be enabled globally in Physics otherwise the flag will be ignored.
        /// </summary>
		CCD = 0x04
    }

    /** @} */
}

//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Physics
     *  @{
     */

    /// <summary>
    /// Collider represents physics geometry that can be in multiple states:
    ///  - Default: Static geometry that physics objects can collide with.
    ///  - Trigger: Static geometry that can't be collided with but will report touch events.
    ///  - Dynamic: Dynamic geometry that is a part of a Rigidbody.A set of colliders defines the shape of the parent 
    ///             rigidbody.
    /// </summary>
    public abstract class Collider : Component
    {
        internal NativeCollider native;
        protected Rigidbody parent;

        [SerializeField]
        internal SerializableData serializableData = new SerializableData();

        /// <summary>
        /// Triggered when some object starts interacting with the collider. Only triggered if proper collision report mode
        /// is turned on.
        /// </summary>
        public event Action<CollisionData> OnCollisionBegin;

        /// <summary>
        /// Triggered for every frame that an object remains interacting with a collider. Only triggered if proper collision
        /// report mode is turned on.
        /// </summary>
        public event Action<CollisionData> OnCollisionStay;

        /// <summary>
        /// Triggered when some object stops interacting with the collider. Only triggered if proper collision report mode
        /// is turned on.
        /// </summary>
        public event Action<CollisionData> OnCollisionEnd;

        /// <summary>
        /// Determines how the collider used. A trigger will not be used for collisions (objects will pass through it),
        /// but collision events will still be reported.
        /// </summary>
        public bool IsTrigger
        {
            get { return serializableData.isTrigger; }
            set
            {
                if (serializableData.isTrigger == value)
                    return;

                serializableData.isTrigger = value;

                if (native != null)
                {
                    native.IsTrigger = value;
                    UpdateParentRigidbody();
                    UpdateTransform();
                }
            }
        }

        /// <summary>
        /// Determines mass of the collider. Only relevant if the collider is part of a rigidbody. Ultimately this will 
        /// determine the total mass, center of mass and inertia tensors of the parent rigidbody(if they're being calculated
        /// automatically).
        /// </summary>
        public float Mass
        {
            get { return serializableData.mass; }
            set
            {
                if (serializableData.mass == value)
                    return;

                serializableData.mass = value;

                if (native != null)
                {
                    native.Mass = value;

                    if (parent != null)
                        parent.UpdateMassDistribution();
                }
            }
        }

        /// <summary>
        /// Physics material that determines how objects hitting the collider behave.
        /// </summary>
        public PhysicsMaterial Material
        {
            get { return serializableData.material; }
            set
            {
                serializableData.material = value;

                if (native != null)
                    native.Material = value;
            }
        }

        /// <summary>
        /// Determines how far apart do two shapes need to be away from each other before the physics runtime starts 
        /// generating repelling impulse for them.This distance will be the sum of contact offsets of the two interacting
        /// objects.If objects are moving fast you can increase this value to start generating the impulse earlier and 
        /// potentially prevent the objects from interpenetrating. This value is in meters. Must be positive and larger
        /// than <see cref="RestOffset"/>.
        /// </summary>
        public float ContactOffset
        {
            get { return serializableData.contactOffset; }
            set
            {
                value = MathEx.Max(0, MathEx.Max(value, RestOffset));

                serializableData.contactOffset = value;

                if (native != null)
                    native.ContactOffset = value;
            }
        }

        /// <summary>
        /// Determines at what distance should two objects resting on one another come to an equilibrium. The value used in the
        /// runtime will be the sum of rest offsets for both interacting objects. This value is in meters. Cannot be larger
        /// than <see cref="ContactOffset"/>
        /// </summary>
        public float RestOffset
        {
            get { return serializableData.restOffset; }
            set
            {
                value = MathEx.Min(value, ContactOffset);

                serializableData.restOffset = value;

                if (native != null)
                    native.RestOffset = value;
            }
        }

        /// <summary>
        /// Determines with which objects will the collider collide. Objects that are allowed to collide with this
        /// object must have the same bits set in their own layers.
        /// </summary>
        public ulong Layer
        {
            get { return serializableData.layer; }
            set
            {
                serializableData.layer = value;

                if (native != null)
                    native.Layer = value;
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
                serializableData.collisionReportMode = value;

                if (native != null)
                    UpdateCollisionReportMode();
            }
        }

        /// <summary>
        /// Checks does the ray hit this collider. 
        /// </summary>
        /// <param name="ray">Ray to check.</param>
        /// <param name="hit">Information about the hit. Valid only if the method returns true.</param>
        /// <param name="maxDist">Maximum distance from the ray origin to search for hits.</param>
        /// <returns>True if the ray has hit the collider.</returns>
        public bool Raycast(Ray ray, out PhysicsQueryHit hit, float maxDist)
        {
            return Raycast(ray.origin, ray.direction, out hit, maxDist);
        }

        /// <summary>
        /// Checks does the ray hit this collider. 
        /// </summary>
        /// <param name="origin">Origin of the ray to check.</param>
        /// <param name="unitDir">Unit direction of the ray to check.</param>
        /// <param name="hit">Information about the hit. Valid only if the method returns true.</param>
        /// <param name="maxDist">Maximum distance from the ray origin to search for hits.</param>
        /// <returns>True if the ray has hit the collider.</returns>
        public bool Raycast(Vector3 origin, Vector3 unitDir, out PhysicsQueryHit hit, float maxDist = float.MaxValue)
        {
            hit = new PhysicsQueryHit();

            if (native == null)
                return false;

            ScriptPhysicsQueryHit scriptHit;
            bool wasHit = native.Raycast(origin, unitDir, out scriptHit, maxDist);

            hit.collider = scriptHit.collider.Component;
            hit.distance = scriptHit.distance;
            hit.normal = scriptHit.normal;
            hit.point = scriptHit.point;
            hit.triangleIdx = scriptHit.triangleIdx;
            hit.uv = scriptHit.uv;

            return wasHit;
        }

        /// <summary>
        /// Creates an internal instance of the collider. Should be overriden by specific collider implementations.
        /// </summary>
        /// <returns>An instance of a specific internal collider implementation. </returns>
        internal abstract NativeCollider CreateCollider();

        /// <summary>
        /// Changes the rigidbody parent of the collider.
        /// </summary>
        /// <param name="rigidbody">New rigidbody to assign as the parent to the collider.</param>
        /// <param name="isInternal">If true the rigidbody will just be changed internally, but parent rigidbody will not be
        ///                          notified.</param>
        internal void SetRigidbody(Rigidbody rigidbody, bool isInternal = false)
        {
		    if (rigidbody == parent)
			    return;

		    if (native != null && !isInternal)
		    {
			    if (parent != null)
                    parent.RemoveCollider(this);

			    NativeRigidbody nativeRigidbody = null;

			    if (rigidbody != null)
                    nativeRigidbody = rigidbody.native;

		        native.Rigidbody = nativeRigidbody;

			    if (rigidbody != null)
				    rigidbody.AddCollider(this);
		    }

		    parent = rigidbody;
		    UpdateCollisionReportMode();
	    }

        /// <summary>
        /// Triggered when the internal collider begins touching another object.
        /// </summary>
        /// <param name="data">Data about the collision.</param>
        internal void DoOnCollisionBegin(CollisionData data)
        {
            if (OnCollisionBegin != null)
                OnCollisionBegin(data);
        }

        /// <summary>
        /// Triggered when the internal collider ends touching another object.
        /// </summary>
        /// <param name="data">Data about the collision.</param>
        internal void DoOnCollisionStay(CollisionData data)
        {
            if (OnCollisionStay != null)
                OnCollisionStay(data);
        }

        /// <summary>
        /// Triggered when the internal collider ends touching another object.
        /// </summary>
        /// <param name="data">Data about the collision.</param>
        internal void DoOnCollisionEnd(CollisionData data)
        {
            if (OnCollisionEnd != null)
                OnCollisionEnd(data);
        }

        /// <summary>
        /// Checks is the provided rigidbody a valid parent for this collider.
        /// 
        /// This is required because certain colliders are limited in how they can be used.
        /// </summary>
        /// <param name="parent">Rigidbody that is the potential parent.</param>
        /// <returns>True if collider can be a part of the rigidbody.</returns>
        protected internal virtual bool IsValidParent(Rigidbody parent)
        {
            return true;
        }

        /// <summary>
        /// Searches the parent scene object hierarchy to find a parent Rigidbody component.
        /// </summary>
        protected void UpdateParentRigidbody()
        {
            if (serializableData.isTrigger)
            {
                SetRigidbody(null);
                return;
            }

            SceneObject currentSO = SceneObject;
            while (currentSO != null)
            {
                Rigidbody parent = currentSO.GetComponent<Rigidbody>();
                if (parent != null)
                {
                    if (currentSO.Active && IsValidParent(parent))
                        SetRigidbody(parent);
                    else
                        SetRigidbody(null);

                    return;
                }

                currentSO = currentSO.Parent;
            }

            // Not found
            SetRigidbody(null);
        }

        /// <summary>
        /// Updates the transform of the internal Collider representation from the transform of the component's scene object.
        /// </summary>
        protected void UpdateTransform()
        {
            Vector3 myScale = SceneObject.Scale;

            if (parent != null)
            {
                Vector3 parentPos = parent.SceneObject.Position;
                Quaternion parentRot = parent.SceneObject.Rotation;

                Vector3 myPos = SceneObject.Position;
                Quaternion myRot = SceneObject.Rotation;

                Vector3 scale = parent.SceneObject.Scale;
                Vector3 invScale = scale;
                if (invScale.x != 0) invScale.x = 1.0f / invScale.x;
                if (invScale.y != 0) invScale.y = 1.0f / invScale.y;
                if (invScale.z != 0) invScale.z = 1.0f / invScale.z;

                Quaternion invRotation = parentRot.Inverse;

                Vector3 relativePos = invRotation.Rotate(myPos - parentPos) * invScale;
                Quaternion relativeRot = invRotation * myRot;

                relativePos = relativePos + myRot.Rotate(serializableData.localPosition * scale);
                relativeRot = relativeRot * serializableData.localRotation;

                native.Position = relativePos;
                native.Rotation = relativeRot;

                parent.UpdateMassDistribution();
            }
            else
            {
                Quaternion myRot = SceneObject.Rotation;
                Vector3 myPos = SceneObject.Position + myRot.Rotate(serializableData.localPosition * myScale);
                myRot = myRot * serializableData.localRotation;

                native.Position = myPos;
                native.Rotation = myRot;
            }

            native.Scale = myScale;
        }

        /// <summary>
        /// Applies the collision report mode to the internal collider depending on the current state.
        /// </summary>
        internal void UpdateCollisionReportMode()
        {
            CollisionReportMode mode = serializableData.collisionReportMode;

            if (parent != null)
                mode = parent.CollisionReportMode;

            if(native != null)
                native.CollisionReportMode = mode;
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
                UpdateParentRigidbody();

            // Don't update the transform if it's due to Physics update since then we can guarantee it will remain at the same
            // relative transform to its parent
            if (Physics.IsUpdateInProgress)
                return;

            if ((flags & (TransformChangedFlags.Parent | TransformChangedFlags.Transform)) != 0)
                UpdateTransform();
        }

        /// <summary>
        /// Destroys the internal collider representation.
        /// </summary>
        private void DestroyNative()
        {
            if (parent != null)
                parent.RemoveCollider(this);

            parent = null;

            if (native != null)
            {
                native.Destroy();
                native = null;
            }
        }

        /// <summary>
        /// Creates the internal representation of the Collider and restores the values saved by the Component.
        /// </summary>
        private void RestoreNative()
        {
            native = CreateCollider();

            native.Component = this;
            native.Position = serializableData.localPosition;
            native.Rotation = serializableData.localRotation;
            native.IsTrigger = serializableData.isTrigger;
            native.Mass = serializableData.mass;
            native.Material = serializableData.material;
            native.ContactOffset = serializableData.contactOffset;
            native.RestOffset = serializableData.restOffset;
            native.Layer = serializableData.layer;

            UpdateParentRigidbody();
            UpdateTransform();
            UpdateCollisionReportMode();
        }

        /// <summary>
        /// Holds all data the collider component needs to persist through serialization.
        /// </summary>
        [SerializeObject]
        internal class SerializableData
        {
            public Vector3 localPosition = Vector3.Zero;
            public Quaternion localRotation = Quaternion.Identity;
            public bool isTrigger = false;
            public float mass = 1.0f;
            public PhysicsMaterial material;
            public float contactOffset = 0.02f;
            public float restOffset = 0.0f;
            public ulong layer = 1;
            public CollisionReportMode collisionReportMode = CollisionReportMode.None;
        }
    }

    /** @} */
}

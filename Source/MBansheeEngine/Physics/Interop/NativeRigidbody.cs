//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Interop
     *  @{
     */

    /// <summary>
    /// Wrapper around the native Rigidbody class.
    /// <see cref="Rigidbody"/>
    /// </summary>
    internal class NativeRigidbody : ScriptObject
    {
        private Rigidbody component;

        /// <summary>
        /// Component that owns the native rigidbody object.
        /// </summary>
        public Rigidbody Component
        {
            get { return component; }
            set { component = value; }
        }

        public Vector3 Position
        {
            get { Vector3 pos; Internal_GetPosition(mCachedPtr, out pos); return pos; }
            set { Quaternion rot = Rotation; Internal_SetTransform(mCachedPtr, ref value, ref rot); }
        }

        public Quaternion Rotation
        {
            get { Quaternion rot; Internal_GetRotation(mCachedPtr, out rot); return rot; }
            set { Vector3 pos = Position; Internal_SetTransform(mCachedPtr, ref pos, ref value); }
        }

        public float Mass
        {
            get { return Internal_GetMass(mCachedPtr); }
            set { Internal_SetMass(mCachedPtr, value); }
        }

        public bool Kinematic
        {
            get { return Internal_GetIsKinematic(mCachedPtr); }
            set { Internal_SetIsKinematic(mCachedPtr, value); }
        }

        public bool Sleeping
        {
            get { return Internal_IsSleeping(mCachedPtr); }
            set
            {
                if (value)
                    Internal_Sleep(mCachedPtr);
                else
                    Internal_WakeUp(mCachedPtr);
            }
        }

        public float SleepThreshold
        {
            get { return Internal_GetSleepThreshold(mCachedPtr); }
            set { Internal_SetSleepThreshold(mCachedPtr, value); }
        }

        public bool UseGravity
        {
            get { return Internal_GetUseGravity(mCachedPtr); }
            set { Internal_SetUseGravity(mCachedPtr, value); }
        }

        public Vector3 Velocity
        {
            get { Vector3 velocity; Internal_GetVelocity(mCachedPtr, out velocity); return velocity; }
            set { Internal_SetVelocity(mCachedPtr, ref value); }
        }

        public Vector3 AngularVelocity
        {
            get { Vector3 velocity; Internal_GetAngularVelocity(mCachedPtr, out velocity); return velocity; }
            set { Internal_SetAngularVelocity(mCachedPtr, ref value); }
        }

        public float Drag
        {
            get { return Internal_GetDrag(mCachedPtr); }
            set { Internal_SetDrag(mCachedPtr, value); }
        }

        public float AngularDrag
        {
            get { return Internal_GetAngularDrag(mCachedPtr); }
            set { Internal_SetAngularDrag(mCachedPtr, value); }
        }

        public Vector3 InertiaTensor
        {
            get { Vector3 tensor; Internal_GetInertiaTensor(mCachedPtr, out tensor); return tensor; }
            set { Internal_SetInertiaTensor(mCachedPtr, ref value); }
        }

        public Vector3 CenterOfMassPosition
        {
            get { Vector3 pos; Internal_GetCenterOfMassPosition(mCachedPtr, out pos); return pos; }
            set { Quaternion rot = CenterOfMassRotation; Internal_SetCenterOfMass(mCachedPtr, ref value, ref rot); }
        }

        public Quaternion CenterOfMassRotation
        {
            get { Quaternion rot; Internal_GetCenterOfMassRotation(mCachedPtr, out rot); return rot; }
            set { Vector3 pos = CenterOfMassPosition; Internal_SetCenterOfMass(mCachedPtr, ref pos, ref value); }
        }

        public float MaxAngularVelocity
        {
            get { return Internal_GetMaxAngularVelocity(mCachedPtr); }
            set { Internal_SetMaxAngularVelocity(mCachedPtr, value); }
        }

        public int PositionSolverCount
        {
            get { return Internal_GetPositionSolverCount(mCachedPtr); }
            set { Internal_SetPositionSolverCount(mCachedPtr, value); }
        }

        public int VelocitySolverCount
        {
            get { return Internal_GetVelocitySolverCount(mCachedPtr); }
            set { Internal_SetVelocitySolverCount(mCachedPtr, value); }
        }

        public RigidbodyFlag Flags
        {
            get { return Internal_GetFlags(mCachedPtr); }
            set { Internal_SetFlags(mCachedPtr, value); }
        }

        public NativeRigidbody(SceneObject linkedSO)
        {
            IntPtr linkedSOPtr = IntPtr.Zero;
            if (linkedSO != null)
                linkedSOPtr = linkedSO.GetCachedPtr();

            Internal_CreateInstance(this, linkedSOPtr);
        }

        public void Destroy()
        {
            Internal_Destroy(mCachedPtr);
        }

        public void Move(Vector3 position)
        {
            Internal_Move(mCachedPtr, ref position);
        }

        public void Rotate(Quaternion rotation)
        {
            Internal_Rotate(mCachedPtr, ref rotation);
        }

        public void AddForce(Vector3 force, ForceMode mode)
        {
            Internal_AddForce(mCachedPtr, ref force, mode);
        }

        public void AddTorque(Vector3 torque, ForceMode mode)
        {
            Internal_AddTorque(mCachedPtr, ref torque, mode);
        }

        public void AddForceAtPoint(Vector3 force, Vector3 position, PointForceMode mode)
        {
            Internal_AddForceAtPoint(mCachedPtr, ref force, ref position, mode);
        }

        public Vector3 GetVelocityAtPoint(Vector3 position)
        {
            Vector3 velocity;
            Internal_GetVelocityAtPoint(mCachedPtr, ref position, out velocity);
            return velocity;
        }

        public void AddCollider(Collider collider)
        {
            if (collider == null || collider.native == null)
                return;

            IntPtr colliderPtr = collider.native.GetCachedPtr();
            Internal_AddCollider(mCachedPtr, colliderPtr);
        }

        public void RemoveCollider(Collider collider)
        {
            if (collider == null)
                return;

            IntPtr colliderPtr = collider.native.GetCachedPtr();
            Internal_RemoveCollider(mCachedPtr, colliderPtr);
        }

        public void RemoveColliders()
        {
            Internal_RemoveColliders(mCachedPtr);
        }

        public void UpdateMassDistribution()
        {
            Internal_UpdateMassDistribution(mCachedPtr);
        }

        private void Internal_DoOnCollisionBegin(ScriptCollisionData scriptCollisionData)
        {
            Component.DoOnCollisionBegin(new CollisionData(scriptCollisionData));
        }

        private void Internal_DoOnCollisionStay(ScriptCollisionData scriptCollisionData)
        {
            Component.DoOnCollisionStay(new CollisionData(scriptCollisionData));
        }

        private void Internal_DoOnCollisionEnd(ScriptCollisionData scriptCollisionData)
        {
            Component.DoOnCollisionEnd(new CollisionData(scriptCollisionData));
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(NativeRigidbody instance, IntPtr linkedSO);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Destroy(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Move(IntPtr thisPtr, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Rotate(IntPtr thisPtr, ref Quaternion rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetPosition(IntPtr thisPtr, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetRotation(IntPtr thisPtr, out Quaternion rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTransform(IntPtr thisPtr, ref Vector3 pos, ref Quaternion rot);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMass(IntPtr thisPtr, float mass);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetMass(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetIsKinematic(IntPtr thisPtr, bool kinematic);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIsKinematic(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsSleeping(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Sleep(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_WakeUp(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetSleepThreshold(IntPtr thisPtr, float threshold);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetSleepThreshold(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetUseGravity(IntPtr thisPtr, bool gravity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetUseGravity(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVelocity(IntPtr thisPtr, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetVelocity(IntPtr thisPtr, out Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAngularVelocity(IntPtr thisPtr, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetAngularVelocity(IntPtr thisPtr, out Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDrag(IntPtr thisPtr, float drag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetDrag(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAngularDrag(IntPtr thisPtr, float drag);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetAngularDrag(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetInertiaTensor(IntPtr thisPtr, ref Vector3 tensor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetInertiaTensor(IntPtr thisPtr, out Vector3 tensor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMaxAngularVelocity(IntPtr thisPtr, float maxVelocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetMaxAngularVelocity(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCenterOfMass(IntPtr thisPtr, ref Vector3 position, ref Quaternion rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetCenterOfMassPosition(IntPtr thisPtr, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetCenterOfMassRotation(IntPtr thisPtr, out Quaternion rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPositionSolverCount(IntPtr thisPtr, int count);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetPositionSolverCount(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVelocitySolverCount(IntPtr thisPtr, int count);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetVelocitySolverCount(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFlags(IntPtr thisPtr, RigidbodyFlag flags);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RigidbodyFlag Internal_GetFlags(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_AddForce(IntPtr thisPtr, ref Vector3 force, ForceMode mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_AddTorque(IntPtr thisPtr, ref Vector3 torque, ForceMode mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_AddForceAtPoint(IntPtr thisPtr, ref Vector3 force, ref Vector3 position, PointForceMode mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetVelocityAtPoint(IntPtr thisPtr, ref Vector3 point, out Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_AddCollider(IntPtr thisPtr, IntPtr collider);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_RemoveCollider(IntPtr thisPtr, IntPtr collider);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_RemoveColliders(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UpdateMassDistribution(IntPtr thisPtr);
    }

    /** @} */
}

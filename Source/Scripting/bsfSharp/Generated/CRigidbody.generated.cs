//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	[ShowInInspector]
	public partial class Rigidbody : Component
	{
		private Rigidbody(bool __dummy0) { }
		protected Rigidbody() { }

		[ShowInInspector]
		[NativeWrapper]
		public float Mass
		{
			get { return Internal_GetMass(mCachedPtr); }
			set { Internal_SetMass(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public bool IsKinematic
		{
			get { return Internal_GetIsKinematic(mCachedPtr); }
			set { Internal_SetIsKinematic(mCachedPtr, value); }
		}

		[NativeWrapper]
		public bool IsSleeping
		{
			get { return Internal_IsSleeping(mCachedPtr); }
		}

		[NativeWrapper]
		public float SleepThreshold
		{
			get { return Internal_GetSleepThreshold(mCachedPtr); }
			set { Internal_SetSleepThreshold(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public bool UseGravity
		{
			get { return Internal_GetUseGravity(mCachedPtr); }
			set { Internal_SetUseGravity(mCachedPtr, value); }
		}

		[NativeWrapper]
		public Vector3 Velocity
		{
			get
			{
				Vector3 temp;
				Internal_GetVelocity(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetVelocity(mCachedPtr, ref value); }
		}

		[NativeWrapper]
		public Vector3 AngularVelocity
		{
			get
			{
				Vector3 temp;
				Internal_GetAngularVelocity(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetAngularVelocity(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Drag
		{
			get { return Internal_GetDrag(mCachedPtr); }
			set { Internal_SetDrag(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float AngularDrag
		{
			get { return Internal_GetAngularDrag(mCachedPtr); }
			set { Internal_SetAngularDrag(mCachedPtr, value); }
		}

		[NativeWrapper]
		public Vector3 InertiaTensor
		{
			get
			{
				Vector3 temp;
				Internal_GetInertiaTensor(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetInertiaTensor(mCachedPtr, ref value); }
		}

		[NativeWrapper]
		public float MaxAngularVelocity
		{
			get { return Internal_GetMaxAngularVelocity(mCachedPtr); }
			set { Internal_SetMaxAngularVelocity(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the rigidbody&apos;s center of mass position. Only relevant if RigibodyFlag::AutoTensors is turned off.
		/// </summary>
		[NativeWrapper]
		public Vector3 CenterOfMassPosition
		{
			get
			{
				Vector3 temp;
				Internal_GetCenterOfMassPosition(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetCenterOfMassPosition(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the rigidbody&apos;s center of mass rotation. Only relevant if RigibodyFlag::AutoTensors is turned off.
		/// </summary>
		[NativeWrapper]
		public Quaternion CenterOfMassRotation
		{
			get
			{
				Quaternion temp;
				Internal_GetCenterOfMassRotation(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetCenterOfMassRotation(mCachedPtr, ref value); }
		}

		[NativeWrapper]
		public int PositionSolverCount
		{
			get { return Internal_GetPositionSolverCount(mCachedPtr); }
			set { Internal_SetPositionSolverCount(mCachedPtr, value); }
		}

		[NativeWrapper]
		public int VelocitySolverCount
		{
			get { return Internal_GetVelocitySolverCount(mCachedPtr); }
			set { Internal_SetVelocitySolverCount(mCachedPtr, value); }
		}

		/// <summary>Sets a value that determines which (if any) collision events are reported.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CollisionReportMode CollisionReportMode
		{
			get { return Internal_GetCollisionReportMode(mCachedPtr); }
			set { Internal_SetCollisionReportMode(mCachedPtr, value); }
		}

		[NativeWrapper]
		public RigidbodyFlag Flags
		{
			get { return Internal_GetFlags(mCachedPtr); }
			set { Internal_SetFlags(mCachedPtr, value); }
		}

		public event Action<CollisionData> OnCollisionBegin;

		public event Action<CollisionData> OnCollisionStay;

		public event Action<CollisionData> OnCollisionEnd;

		public void Move(Vector3 position)
		{
			Internal_Move(mCachedPtr, ref position);
		}

		public void Rotate(Quaternion rotation)
		{
			Internal_Rotate(mCachedPtr, ref rotation);
		}

		public void Sleep()
		{
			Internal_Sleep(mCachedPtr);
		}

		public void WakeUp()
		{
			Internal_WakeUp(mCachedPtr);
		}

		public void AddForce(Vector3 force, ForceMode mode = ForceMode.Force)
		{
			Internal_AddForce(mCachedPtr, ref force, mode);
		}

		public void AddTorque(Vector3 torque, ForceMode mode = ForceMode.Force)
		{
			Internal_AddTorque(mCachedPtr, ref torque, mode);
		}

		public void AddForceAtPoint(Vector3 force, Vector3 position, PointForceMode mode = PointForceMode.Force)
		{
			Internal_AddForceAtPoint(mCachedPtr, ref force, ref position, mode);
		}

		public Vector3 GetVelocityAtPoint(Vector3 point)
		{
			Vector3 temp;
			Internal_GetVelocityAtPoint(mCachedPtr, ref point, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Move(IntPtr thisPtr, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Rotate(IntPtr thisPtr, ref Quaternion rotation);
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
		private static extern void Internal_GetVelocity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAngularVelocity(IntPtr thisPtr, ref Vector3 velocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetAngularVelocity(IntPtr thisPtr, out Vector3 __output);
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
		private static extern void Internal_GetInertiaTensor(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxAngularVelocity(IntPtr thisPtr, float maxVelocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMaxAngularVelocity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCenterOfMassPosition(IntPtr thisPtr, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCenterOfMassPosition(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCenterOfMassRotation(IntPtr thisPtr, ref Quaternion rotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCenterOfMassRotation(IntPtr thisPtr, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPositionSolverCount(IntPtr thisPtr, int count);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetPositionSolverCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVelocitySolverCount(IntPtr thisPtr, int count);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetVelocitySolverCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCollisionReportMode(IntPtr thisPtr, CollisionReportMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CollisionReportMode Internal_GetCollisionReportMode(IntPtr thisPtr);
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
		private static extern void Internal_GetVelocityAtPoint(IntPtr thisPtr, ref Vector3 point, out Vector3 __output);
		private void Internal_OnCollisionBegin(ref CollisionData p0)
		{
			OnCollisionBegin?.Invoke(p0);
		}
		private void Internal_OnCollisionStay(ref CollisionData p0)
		{
			OnCollisionStay?.Invoke(p0);
		}
		private void Internal_OnCollisionEnd(ref CollisionData p0)
		{
			OnCollisionEnd?.Invoke(p0);
		}
	}

	/** @} */
}

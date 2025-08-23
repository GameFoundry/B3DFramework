//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>
	/// Base class for all Joint types. Joints constrain how two rigidbodies move relative to one another (for example a door 
	/// hinge). One of the bodies in the joint must always be movable (non-kinematic).
	/// </summary>
	[ShowInInspector]
	public partial class Joint : Component
	{
		private Joint(bool __dummy0) { }
		protected Joint() { }

		/// <summary>
		/// Determines the maximum force the joint can apply before breaking. Broken joints no longer participate in physics 
		/// simulation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float BreakForce
		{
			get { return Internal_GetBreakForce(mCachedPtr); }
			set { Internal_SetBreakForce(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum torque the joint can apply before breaking. Broken joints no longer participate in physics 
		/// simulation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float BreakTorque
		{
			get { return Internal_GetBreakTorque(mCachedPtr); }
			set { Internal_SetBreakTorque(mCachedPtr, value); }
		}

		/// <summary>Determines whether collision between the two bodies managed by the joint are enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableCollision
		{
			get { return Internal_GetEnableCollision(mCachedPtr); }
			set { Internal_SetEnableCollision(mCachedPtr, value); }
		}

		/// <summary>Triggered when the joint&apos;s break force or torque is exceeded.</summary>
		public event Action OnJointBreak;

		/// <summary>Determines a body managed by the joint. One of the bodies must be movable (non-kinematic).</summary>
		public void SetBody(JointBody body, Rigidbody value)
		{
			Internal_SetBody(mCachedPtr, body, value);
		}

		public Rigidbody GetBody(JointBody body)
		{
			return Internal_GetBody(mCachedPtr, body);
		}

		/// <summary>Returns the position at which the body is anchored to the joint, relative to the body.</summary>
		public Vector3 GetPosition(JointBody body)
		{
			Vector3 temp;
			Internal_GetRelativeBodyPosition(mCachedPtr, body, out temp);
			return temp;
		}

		/// <summary>Returns the rotation at which the body is anchored to the joint, relative to the body.</summary>
		public Quaternion GetRotation(JointBody body)
		{
			Quaternion temp;
			Internal_GetRelativeBodyRotation(mCachedPtr, body, out temp);
			return temp;
		}

		/// <summary>Sets the position and rotation at which the body is anchored to the joint, relative to the body.</summary>
		public void SetTransform(JointBody body, Vector3 position, Quaternion rotation)
		{
			Internal_SetRelativeBodyTransform(mCachedPtr, body, ref position, ref rotation);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBody(IntPtr thisPtr, JointBody body, Rigidbody value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Rigidbody Internal_GetBody(IntPtr thisPtr, JointBody body);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetRelativeBodyPosition(IntPtr thisPtr, JointBody body, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetRelativeBodyRotation(IntPtr thisPtr, JointBody body, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRelativeBodyTransform(IntPtr thisPtr, JointBody body, ref Vector3 position, ref Quaternion rotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBreakForce(IntPtr thisPtr, float force);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetBreakForce(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBreakTorque(IntPtr thisPtr, float torque);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetBreakTorque(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableCollision(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableCollision(IntPtr thisPtr);
		private void Internal_OnJointBreak()
		{
			OnJointBreak?.Invoke();
		}
	}

	/** @} */
}

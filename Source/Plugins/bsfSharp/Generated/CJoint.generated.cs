//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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
			get { return Internal_getBreakForce(mCachedPtr); }
			set { Internal_setBreakForce(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum torque the joint can apply before breaking. Broken joints no longer participate in physics  
		/// simulation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float BreakTorque
		{
			get { return Internal_getBreakTorque(mCachedPtr); }
			set { Internal_setBreakTorque(mCachedPtr, value); }
		}

		/// <summary>Determines whether collision between the two bodies managed by the joint are enabled.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableCollision
		{
			get { return Internal_getEnableCollision(mCachedPtr); }
			set { Internal_setEnableCollision(mCachedPtr, value); }
		}

		/// <summary>Triggered when the joint's break force or torque is exceeded.</summary>
		public event Action OnJointBreak;

		/// <summary>Determines a body managed by the joint. One of the bodies must be movable (non-kinematic).</summary>
		public Rigidbody GetBody(JointBody body)
		{
			return Internal_getBody(mCachedPtr, body);
		}

		/// <summary>Determines a body managed by the joint. One of the bodies must be movable (non-kinematic).</summary>
		public void SetBody(JointBody body, Rigidbody value)
		{
			Internal_setBody(mCachedPtr, body, value);
		}

		/// <summary>Returns the position relative to the body, at which the body is anchored to the joint.</summary>
		public Vector3 GetPosition(JointBody body)
		{
			Vector3 temp;
			Internal_getPosition(mCachedPtr, body, out temp);
			return temp;
		}

		/// <summary>Returns the rotation relative to the body, at which the body is anchored to the joint.</summary>
		public Quaternion GetRotation(JointBody body)
		{
			Quaternion temp;
			Internal_getRotation(mCachedPtr, body, out temp);
			return temp;
		}

		/// <summary>Sets the position and rotation relative to the body, at which the body is anchored to the joint.</summary>
		public void SetTransform(JointBody body, Vector3 position, Quaternion rotation)
		{
			Internal_setTransform(mCachedPtr, body, ref position, ref rotation);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Rigidbody Internal_getBody(IntPtr thisPtr, JointBody body);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setBody(IntPtr thisPtr, JointBody body, Rigidbody value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPosition(IntPtr thisPtr, JointBody body, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getRotation(IntPtr thisPtr, JointBody body, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setTransform(IntPtr thisPtr, JointBody body, ref Vector3 position, ref Quaternion rotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getBreakForce(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setBreakForce(IntPtr thisPtr, float force);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getBreakTorque(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setBreakTorque(IntPtr thisPtr, float torque);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getEnableCollision(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setEnableCollision(IntPtr thisPtr, bool value);
		private void Internal_onJointBreak()
		{
			OnJointBreak?.Invoke();
		}
	}

	/** @} */
}

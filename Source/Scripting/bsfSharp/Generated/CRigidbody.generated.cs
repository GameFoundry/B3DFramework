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
	/// Rigidbody is a dynamic physics object that can be moved using forces (or directly). It will interact with other static 
	/// and dynamic physics objects in the scene accordingly (it will push other non-kinematic rigidbodies,  and collide with 
	/// static objects).
	///
	/// The shape and mass of a rigidbody is governed by its colliders. You must attach at least one collider for the  
	/// rigidbody to be valid.
	/// </summary>
	[ShowInInspector]
	public partial class Rigidbody : Component
	{
		private Rigidbody(bool __dummy0) { }
		protected Rigidbody() { }

		/// <summary>
		/// Determines the mass of the object and all of its collider shapes. Only relevant if RigidbodyFlag::AutoMass or  
		/// RigidbodyFlag::AutoTensors is turned off. Value of zero means the object is immovable (but can be rotated).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Mass
		{
			get { return Internal_getMass(mCachedPtr); }
			set { Internal_setMass(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines if the body is kinematic. Kinematic body will not move in response to external forces (for example 
		/// gravity, or another object pushing it), essentially behaving like collider. Unlike a collider though, you can still 
		/// move the object and have other dynamic objects respond correctly (meaning it will push other objects).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool IsKinematic
		{
			get { return Internal_getIsKinematic(mCachedPtr); }
			set { Internal_setIsKinematic(mCachedPtr, value); }
		}

		/// <summary>
		/// Checks if the body is sleeping. Objects that aren&apos;t moved/rotated for a while are put to sleep to reduce load on 
		/// the physics system.
		/// </summary>
		[NativeWrapper]
		public bool IsSleeping
		{
			get { return Internal_isSleeping(mCachedPtr); }
		}

		/// <summary>
		/// Determines a threshold of force and torque under which the object will be considered to be put to sleep.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float SleepThreshold
		{
			get { return Internal_getSleepThreshold(mCachedPtr); }
			set { Internal_setSleepThreshold(mCachedPtr, value); }
		}

		/// <summary>Determines whether or not the rigidbody will have the global gravity force applied to it.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool UseGravity
		{
			get { return Internal_getUseGravity(mCachedPtr); }
			set { Internal_setUseGravity(mCachedPtr, value); }
		}

		/// <summary>Determines the linear velocity of the body.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Velocity
		{
			get
			{
				Vector3 temp;
				Internal_getVelocity(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setVelocity(mCachedPtr, ref value); }
		}

		/// <summary>Determines the angular velocity of the body.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 AngularVelocity
		{
			get
			{
				Vector3 temp;
				Internal_getAngularVelocity(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setAngularVelocity(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the linear drag of the body. Higher drag values means the object resists linear movement more.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Drag
		{
			get { return Internal_getDrag(mCachedPtr); }
			set { Internal_setDrag(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the angular drag of the body. Higher drag values means the object resists angular movement more.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float AngularDrag
		{
			get { return Internal_getAngularDrag(mCachedPtr); }
			set { Internal_setAngularDrag(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the inertia tensor in local mass space. Inertia tensor determines how difficult is to rotate the object. 
		/// Values of zero in the inertia tensor mean the object will be unable to rotate around a specific axis.  Only relevant 
		/// if RigidbodyFlag::AutoTensors is turned off.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 InertiaTensor
		{
			get
			{
				Vector3 temp;
				Internal_getInertiaTensor(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setInertiaTensor(mCachedPtr, ref value); }
		}

		/// <summary>Determines the maximum angular velocity of the rigidbody. Velocity will be clamped to this value.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MaxAngularVelocity
		{
			get { return Internal_getMaxAngularVelocity(mCachedPtr); }
			set { Internal_setMaxAngularVelocity(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the rigidbody&apos;s center of mass position. Only relevant if RigibodyFlag::AutoTensors is turned off.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 CenterOfMassPosition
		{
			get
			{
				Vector3 temp;
				Internal_getCenterOfMassPosition(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setCenterOfMassPosition(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the rigidbody&apos;s center of mass rotation. Only relevant if RigibodyFlag::AutoTensors is turned off.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Quaternion CenterOfMassRotation
		{
			get
			{
				Quaternion temp;
				Internal_getCenterOfMassRotation(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setCenterOfMassRotation(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the number of iterations to use when solving for position. Higher values can improve precision and  
		/// numerical stability of the simulation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int PositionSolverCount
		{
			get { return Internal_getPositionSolverCount(mCachedPtr); }
			set { Internal_setPositionSolverCount(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the number of iterations to use when solving for velocity. Higher values can improve precision and  
		/// numerical stability of the simulation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int VelocitySolverCount
		{
			get { return Internal_getVelocitySolverCount(mCachedPtr); }
			set { Internal_setVelocitySolverCount(mCachedPtr, value); }
		}

		/// <summary>Sets a value that determines which (if any) collision events are reported.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CollisionReportMode CollisionReportMode
		{
			get { return Internal_getCollisionReportMode(mCachedPtr); }
			set { Internal_setCollisionReportMode(mCachedPtr, value); }
		}

		/// <summary>Flags that control the behaviour of the rigidbody.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RigidbodyFlag Flags
		{
			get { return Internal_getFlags(mCachedPtr); }
			set { Internal_setFlags(mCachedPtr, value); }
		}

		/// <summary>Triggered when one of the colliders owned by the rigidbody starts colliding with another object.</summary>
		public event Action<CollisionData> OnCollisionBegin;

		/// <summary>Triggered when a previously colliding collider stays in collision. Triggered once per frame.</summary>
		public event Action<CollisionData> OnCollisionStay;

		/// <summary>Triggered when one of the colliders owned by the rigidbody stops colliding with another object.</summary>
		public event Action<CollisionData> OnCollisionEnd;

		/// <summary>
		/// Moves the rigidbody to a specific position. This method will ensure physically correct movement, meaning the body 
		/// will collide with other objects along the way.
		/// </summary>
		public void Move(Vector3 position)
		{
			Internal_move(mCachedPtr, ref position);
		}

		/// <summary>
		/// Rotates the rigidbody. This method will ensure physically correct rotation, meaning the body will collide with  other 
		/// objects along the way.
		/// </summary>
		public void Rotate(Quaternion rotation)
		{
			Internal_rotate(mCachedPtr, ref rotation);
		}

		/// <summary>
		/// Forces the object to sleep. Useful if you know the object will not move in any significant way for a while.
		/// </summary>
		public void Sleep()
		{
			Internal_sleep(mCachedPtr);
		}

		/// <summary>
		/// Wakes an object up. Useful if you modified properties of this object, and potentially surrounding objects which might 
		/// result in the object being moved by physics (although the physics system will automatically wake the object up for 
		/// majority of such cases).
		/// </summary>
		public void WakeUp()
		{
			Internal_wakeUp(mCachedPtr);
		}

		/// <summary>Applies a force to the center of the mass of the rigidbody. This will produce linear momentum.</summary>
		/// <param name="force">Force to apply.</param>
		/// <param name="mode">Determines what is the type of <paramref name="force"/>.</param>
		public void AddForce(Vector3 force, ForceMode mode = ForceMode.Force)
		{
			Internal_addForce(mCachedPtr, ref force, mode);
		}

		/// <summary>Applies a torque to the rigidbody. This will produce angular momentum.</summary>
		/// <param name="torque">Torque to apply.</param>
		/// <param name="mode">Determines what is the type of <paramref name="torque"/>.</param>
		public void AddTorque(Vector3 torque, ForceMode mode = ForceMode.Force)
		{
			Internal_addTorque(mCachedPtr, ref torque, mode);
		}

		/// <summary>
		/// Applies a force to a specific point on the rigidbody. This will in most cases produce both linear and angular 
		/// momentum.
		/// </summary>
		/// <param name="force">Force to apply.</param>
		/// <param name="position">World position to apply the force at.</param>
		/// <param name="mode">Determines what is the type of <paramref name="force"/>.</param>
		public void AddForceAtPoint(Vector3 force, Vector3 position, PointForceMode mode = PointForceMode.Force)
		{
			Internal_addForceAtPoint(mCachedPtr, ref force, ref position, mode);
		}

		/// <summary>Returns the total (linear + angular) velocity at a specific point.</summary>
		/// <param name="point">Point in world space.</param>
		/// <returns>Total velocity of the point.</returns>
		public Vector3 GetVelocityAtPoint(Vector3 point)
		{
			Vector3 temp;
			Internal_getVelocityAtPoint(mCachedPtr, ref point, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_move(IntPtr thisPtr, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_rotate(IntPtr thisPtr, ref Quaternion rotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMass(IntPtr thisPtr, float mass);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMass(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setIsKinematic(IntPtr thisPtr, bool kinematic);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getIsKinematic(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isSleeping(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sleep(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_wakeUp(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSleepThreshold(IntPtr thisPtr, float threshold);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getSleepThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setUseGravity(IntPtr thisPtr, bool gravity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getUseGravity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setVelocity(IntPtr thisPtr, ref Vector3 velocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getVelocity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setAngularVelocity(IntPtr thisPtr, ref Vector3 velocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getAngularVelocity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setDrag(IntPtr thisPtr, float drag);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getDrag(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setAngularDrag(IntPtr thisPtr, float drag);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getAngularDrag(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setInertiaTensor(IntPtr thisPtr, ref Vector3 tensor);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getInertiaTensor(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMaxAngularVelocity(IntPtr thisPtr, float maxVelocity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMaxAngularVelocity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCenterOfMassPosition(IntPtr thisPtr, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getCenterOfMassPosition(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCenterOfMassRotation(IntPtr thisPtr, ref Quaternion rotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getCenterOfMassRotation(IntPtr thisPtr, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPositionSolverCount(IntPtr thisPtr, int count);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getPositionSolverCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setVelocitySolverCount(IntPtr thisPtr, int count);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getVelocitySolverCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCollisionReportMode(IntPtr thisPtr, CollisionReportMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CollisionReportMode Internal_getCollisionReportMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFlags(IntPtr thisPtr, RigidbodyFlag flags);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RigidbodyFlag Internal_getFlags(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_addForce(IntPtr thisPtr, ref Vector3 force, ForceMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_addTorque(IntPtr thisPtr, ref Vector3 torque, ForceMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_addForceAtPoint(IntPtr thisPtr, ref Vector3 force, ref Vector3 position, PointForceMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getVelocityAtPoint(IntPtr thisPtr, ref Vector3 point, out Vector3 __output);
		private void Internal_onCollisionBegin(ref CollisionData p0)
		{
			OnCollisionBegin?.Invoke(p0);
		}
		private void Internal_onCollisionStay(ref CollisionData p0)
		{
			OnCollisionStay?.Invoke(p0);
		}
		private void Internal_onCollisionEnd(ref CollisionData p0)
		{
			OnCollisionEnd?.Invoke(p0);
		}
	}

	/** @} */
}

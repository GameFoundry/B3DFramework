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
	/// Represents the most customizable type of joint. This joint type can be used to create all other built-in joint  types, 
	/// and to design your own custom ones, but is less intuitive to use. Allows a specification of a linear  constraint (for 
	/// example for slider), twist constraint (rotating around X) and swing constraint (rotating around Y and Z). It also 
	/// allows you to constrain limits to only specific axes or completely lock specific axes.
	/// </summary>
	[ShowInInspector]
	public partial class D6Joint : Joint
	{
		private D6Joint(bool __dummy0) { }
		protected D6Joint() { }

		/// <summary>Returns the current rotation of the joint around the X axis.</summary>
		[NativeWrapper]
		public Radian Twist
		{
			get
			{
				Radian temp;
				Internal_getTwist(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns the current rotation of the joint around the Y axis.</summary>
		[NativeWrapper]
		public Radian SwingY
		{
			get
			{
				Radian temp;
				Internal_getSwingY(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns the current rotation of the joint around the Z axis.</summary>
		[NativeWrapper]
		public Radian SwingZ
		{
			get
			{
				Radian temp;
				Internal_getSwingZ(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Determines the linear limit used for constraining translation degrees of freedom.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public LimitLinear LimitLinear
		{
			get
			{
				LimitLinear temp;
				Internal_getLimitLinear(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setLimitLinear(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the angular limit used for constraining the twist (rotation around X) degree of freedom.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public LimitAngularRange LimitTwist
		{
			get
			{
				LimitAngularRange temp;
				Internal_getLimitTwist(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setLimitTwist(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the cone limit used for constraining the swing (rotation around Y and Z) degree of freedom.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public LimitConeRange LimitSwing
		{
			get
			{
				LimitConeRange temp;
				Internal_getLimitSwing(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setLimitSwing(mCachedPtr, ref value); }
		}

		/// <summary>Returns the drive&apos;s target position relative to the joint&apos;s first body.</summary>
		[NativeWrapper]
		public Vector3 DrivePosition
		{
			get
			{
				Vector3 temp;
				Internal_getDrivePosition(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns the drive&apos;s target rotation relative to the joint&apos;s first body.</summary>
		[NativeWrapper]
		public Quaternion DriveRotation
		{
			get
			{
				Quaternion temp;
				Internal_getDriveRotation(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns the drive&apos;s target linear velocity.</summary>
		[NativeWrapper]
		public Vector3 DriveLinearVelocity
		{
			get
			{
				Vector3 temp;
				Internal_getDriveLinearVelocity(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns the drive&apos;s target angular velocity.</summary>
		[NativeWrapper]
		public Vector3 DriveAngularVelocity
		{
			get
			{
				Vector3 temp;
				Internal_getDriveAngularVelocity(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns motion constraint for the specified axis.</summary>
		public D6JointMotion GetMotion(D6JointAxis axis)
		{
			return Internal_getMotion(mCachedPtr, axis);
		}

		/// <summary>
		/// Allows you to constrain motion of the specified axis. Be aware that when setting drives for a specific axis you must 
		/// also take care not to constrain its motion in a conflicting way (for example you cannot add a drive that moves the 
		/// joint on X axis, and then lock the X axis).
		///
		/// Unlocking translations degrees of freedom allows the bodies to move along the subset of the unlocked axes. (for 
		/// example unlocking just one translational axis is the equivalent of a slider joint.)
		///
		/// Angular degrees of freedom are partitioned as twist (around X axis) and swing (around Y and Z axes). Different 
		/// effects can be achieves by unlocking their various combinations:  - If a single degree of angular freedom is unlocked 
		/// it should be the twist degree as it has extra options for that case (for example for a hinge joint). - If both swing 
		/// degrees are unlocked but twist is locked the result is a zero-twist joint.  - If one swing and one twist degree of 
		/// freedom are unlocked the result is a zero-swing joint (for example an arm attached at the elbow) - If all angular 
		/// degrees of freedom are unlocked the result is the same as the spherical joint.
		/// </summary>
		public void SetMotion(D6JointAxis axis, D6JointMotion motion)
		{
			Internal_setMotion(mCachedPtr, axis, motion);
		}

		/// <summary>
		/// Determines a drive that will attempt to move the specified degree(s) of freedom to the wanted position and velocity.
		/// </summary>
		public D6JointDrive GetDrive(D6JointDriveType type)
		{
			D6JointDrive temp;
			Internal_getDrive(mCachedPtr, type, out temp);
			return temp;
		}

		/// <summary>
		/// Determines a drive that will attempt to move the specified degree(s) of freedom to the wanted position and velocity.
		/// </summary>
		public void SetDrive(D6JointDriveType type, D6JointDrive drive)
		{
			Internal_setDrive(mCachedPtr, type, ref drive);
		}

		/// <summary>Sets the drive&apos;s target position and rotation relative to the joint&apos;s first body.</summary>
		public void SetDriveTransform(Vector3 position, Quaternion rotation)
		{
			Internal_setDriveTransform(mCachedPtr, ref position, ref rotation);
		}

		/// <summary>Sets the drive&apos;s target linear and angular velocities.</summary>
		public void SetDriveVelocity(Vector3 linear, Vector3 angular)
		{
			Internal_setDriveVelocity(mCachedPtr, ref linear, ref angular);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern D6JointMotion Internal_getMotion(IntPtr thisPtr, D6JointAxis axis);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMotion(IntPtr thisPtr, D6JointAxis axis, D6JointMotion motion);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getTwist(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getSwingY(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getSwingZ(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getLimitLinear(IntPtr thisPtr, out LimitLinear __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLimitLinear(IntPtr thisPtr, ref LimitLinear limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getLimitTwist(IntPtr thisPtr, out LimitAngularRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLimitTwist(IntPtr thisPtr, ref LimitAngularRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getLimitSwing(IntPtr thisPtr, out LimitConeRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLimitSwing(IntPtr thisPtr, ref LimitConeRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getDrive(IntPtr thisPtr, D6JointDriveType type, out D6JointDrive __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setDrive(IntPtr thisPtr, D6JointDriveType type, ref D6JointDrive drive);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getDrivePosition(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getDriveRotation(IntPtr thisPtr, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setDriveTransform(IntPtr thisPtr, ref Vector3 position, ref Quaternion rotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getDriveLinearVelocity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getDriveAngularVelocity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setDriveVelocity(IntPtr thisPtr, ref Vector3 linear, ref Vector3 angular);
	}

	/** @} */
}

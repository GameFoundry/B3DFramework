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
	public partial class D6Joint : Joint
	{
		private D6Joint(bool __dummy0) { }
		protected D6Joint() { }

		[NativeWrapper]
		public Radian Twist
		{
			get
			{
				Radian temp;
				Internal_GetTwist(mCachedPtr, out temp);
				return temp;
			}
		}

		[NativeWrapper]
		public Radian SwingY
		{
			get
			{
				Radian temp;
				Internal_GetSwingY(mCachedPtr, out temp);
				return temp;
			}
		}

		[NativeWrapper]
		public Radian SwingZ
		{
			get
			{
				Radian temp;
				Internal_GetSwingZ(mCachedPtr, out temp);
				return temp;
			}
		}

		[ShowInInspector]
		[NativeWrapper]
		public LimitLinear LimitLinear
		{
			get
			{
				LimitLinear temp;
				Internal_GetLimitLinear(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetLimitLinear(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public LimitAngularRange LimitTwist
		{
			get
			{
				LimitAngularRange temp;
				Internal_GetLimitTwist(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetLimitTwist(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public LimitConeRange LimitSwing
		{
			get
			{
				LimitConeRange temp;
				Internal_GetLimitSwing(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetLimitSwing(mCachedPtr, ref value); }
		}

		[NativeWrapper]
		public Vector3 DrivePosition
		{
			get
			{
				Vector3 temp;
				Internal_GetDrivePosition(mCachedPtr, out temp);
				return temp;
			}
		}

		[NativeWrapper]
		public Quaternion DriveRotation
		{
			get
			{
				Quaternion temp;
				Internal_GetDriveRotation(mCachedPtr, out temp);
				return temp;
			}
		}

		[NativeWrapper]
		public Vector3 DriveLinearVelocity
		{
			get
			{
				Vector3 temp;
				Internal_GetDriveLinearVelocity(mCachedPtr, out temp);
				return temp;
			}
		}

		[NativeWrapper]
		public Vector3 DriveAngularVelocity
		{
			get
			{
				Vector3 temp;
				Internal_GetDriveAngularVelocity(mCachedPtr, out temp);
				return temp;
			}
		}

		public D6JointMotion GetMotion(D6JointAxis axis)
		{
			return Internal_GetMotion(mCachedPtr, axis);
		}

		public void SetMotion(D6JointAxis axis, D6JointMotion motion)
		{
			Internal_SetMotion(mCachedPtr, axis, motion);
		}

		public D6JointDrive GetDrive(D6JointDriveType type)
		{
			D6JointDrive temp;
			Internal_GetDrive(mCachedPtr, type, out temp);
			return temp;
		}

		public void SetDrive(D6JointDriveType type, D6JointDrive drive)
		{
			Internal_SetDrive(mCachedPtr, type, ref drive);
		}

		public void SetDriveTransform(Vector3 position, Quaternion rotation)
		{
			Internal_SetDriveTransform(mCachedPtr, ref position, ref rotation);
		}

		public void SetDriveVelocity(Vector3 linear, Vector3 angular)
		{
			Internal_SetDriveVelocity(mCachedPtr, ref linear, ref angular);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern D6JointMotion Internal_GetMotion(IntPtr thisPtr, D6JointAxis axis);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMotion(IntPtr thisPtr, D6JointAxis axis, D6JointMotion motion);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetTwist(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSwingY(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSwingZ(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetLimitLinear(IntPtr thisPtr, out LimitLinear __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLimitLinear(IntPtr thisPtr, ref LimitLinear limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetLimitTwist(IntPtr thisPtr, out LimitAngularRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLimitTwist(IntPtr thisPtr, ref LimitAngularRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetLimitSwing(IntPtr thisPtr, out LimitConeRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLimitSwing(IntPtr thisPtr, ref LimitConeRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetDrive(IntPtr thisPtr, D6JointDriveType type, out D6JointDrive __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDrive(IntPtr thisPtr, D6JointDriveType type, ref D6JointDrive drive);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetDrivePosition(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetDriveRotation(IntPtr thisPtr, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDriveTransform(IntPtr thisPtr, ref Vector3 position, ref Quaternion rotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetDriveLinearVelocity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetDriveAngularVelocity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDriveVelocity(IntPtr thisPtr, ref Vector3 linear, ref Vector3 angular);
	}

	/** @} */
}

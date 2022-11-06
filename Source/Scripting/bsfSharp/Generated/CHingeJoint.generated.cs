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
	public partial class HingeJoint : Joint
	{
		private HingeJoint(bool __dummy0) { }
		protected HingeJoint() { }

		[NativeWrapper]
		public Radian Angle
		{
			get
			{
				Radian temp;
				Internal_GetAngle(mCachedPtr, out temp);
				return temp;
			}
		}

		[NativeWrapper]
		public float Speed
		{
			get { return Internal_GetSpeed(mCachedPtr); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public LimitAngularRange Limit
		{
			get
			{
				LimitAngularRange temp;
				Internal_GetLimit(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetLimit(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public HingeJointDrive Drive
		{
			get
			{
				HingeJointDrive temp;
				Internal_GetDrive(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetDrive(mCachedPtr, ref value); }
		}

		public void SetFlag(HingeJointFlag flag, bool enabled)
		{
			Internal_SetFlag(mCachedPtr, flag, enabled);
		}

		public bool HasFlag(HingeJointFlag flag)
		{
			return Internal_HasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetAngle(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetLimit(IntPtr thisPtr, out LimitAngularRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLimit(IntPtr thisPtr, ref LimitAngularRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetDrive(IntPtr thisPtr, out HingeJointDrive __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDrive(IntPtr thisPtr, ref HingeJointDrive drive);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlag(IntPtr thisPtr, HingeJointFlag flag, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasFlag(IntPtr thisPtr, HingeJointFlag flag);
	}

	/** @} */
}

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
	public partial class SliderJoint : Joint
	{
		private SliderJoint(bool __dummy0) { }
		protected SliderJoint() { }

		[NativeWrapper]
		public float Position
		{
			get { return Internal_GetPosition(mCachedPtr); }
		}

		[NativeWrapper]
		public float Speed
		{
			get { return Internal_GetSpeed(mCachedPtr); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public LimitLinearRange Limit
		{
			get
			{
				LimitLinearRange temp;
				Internal_GetLimit(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetLimit(mCachedPtr, ref value); }
		}

		public void SetFlag(SliderJointFlag flag, bool enabled)
		{
			Internal_SetFlag(mCachedPtr, flag, enabled);
		}

		public bool HasFlag(SliderJointFlag flag)
		{
			return Internal_HasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetPosition(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetLimit(IntPtr thisPtr, out LimitLinearRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLimit(IntPtr thisPtr, ref LimitLinearRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlag(IntPtr thisPtr, SliderJointFlag flag, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasFlag(IntPtr thisPtr, SliderJointFlag flag);
	}

	/** @} */
}

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
	public partial class DistanceJoint : Joint
	{
		private DistanceJoint(bool __dummy0) { }
		protected DistanceJoint() { }

		[NativeWrapper]
		public float Distance
		{
			get { return Internal_GetDistance(mCachedPtr); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float MinDistance
		{
			get { return Internal_GetMinDistance(mCachedPtr); }
			set { Internal_SetMinDistance(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float MaxDistance
		{
			get { return Internal_GetMaxDistance(mCachedPtr); }
			set { Internal_SetMaxDistance(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Tolerance
		{
			get { return Internal_GetTolerance(mCachedPtr); }
			set { Internal_SetTolerance(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public Spring Spring
		{
			get
			{
				Spring temp;
				Internal_GetSpring(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSpring(mCachedPtr, ref value); }
		}

		public void SetFlag(DistanceJointFlag flag, bool enabled)
		{
			Internal_SetFlag(mCachedPtr, flag, enabled);
		}

		public bool HasFlag(DistanceJointFlag flag)
		{
			return Internal_HasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMinDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMinDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMaxDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetTolerance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTolerance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSpring(IntPtr thisPtr, out Spring __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpring(IntPtr thisPtr, ref Spring value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlag(IntPtr thisPtr, DistanceJointFlag flag, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasFlag(IntPtr thisPtr, DistanceJointFlag flag);
	}

	/** @} */
}

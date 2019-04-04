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
	/// Joint that removes all but a single translational degree of freedom. Bodies are allowed to move along a single axis.
	/// </summary>
	[ShowInInspector]
	public partial class SliderJoint : Joint
	{
		private SliderJoint(bool __dummy0) { }
		protected SliderJoint() { }

		/// <summary>Returns the current position of the slider.</summary>
		[NativeWrapper]
		public float Position
		{
			get { return Internal_getPosition(mCachedPtr); }
		}

		/// <summary>Returns the current speed of the slider.</summary>
		[NativeWrapper]
		public float Speed
		{
			get { return Internal_getSpeed(mCachedPtr); }
		}

		/// <summary>
		/// Determines a limit that constrains the movement of the joint to a specific minimum and maximum distance. You must 
		/// enable the limit flag on the joint in order for this to be recognized.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public LimitLinearRange Limit
		{
			get
			{
				LimitLinearRange temp;
				Internal_getLimit(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setLimit(mCachedPtr, ref value); }
		}

		/// <summary>Enables or disables a flag that controls the joint&apos;s behaviour.</summary>
		public void SetFlag(SliderJointFlag flag, bool enabled)
		{
			Internal_setFlag(mCachedPtr, flag, enabled);
		}

		/// <summary>Checks is the specified flag enabled.</summary>
		public bool HasFlag(SliderJointFlag flag)
		{
			return Internal_hasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getPosition(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getLimit(IntPtr thisPtr, out LimitLinearRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLimit(IntPtr thisPtr, ref LimitLinearRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFlag(IntPtr thisPtr, SliderJointFlag flag, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_hasFlag(IntPtr thisPtr, SliderJointFlag flag);
	}

	/** @} */
}

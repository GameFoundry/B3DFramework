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

	/// <summary>A joint that maintains an upper or lower (or both) bound on the distance between two bodies.</summary>
	[ShowInInspector]
	public partial class DistanceJoint : Joint
	{
		private DistanceJoint(bool __dummy0) { }
		protected DistanceJoint() { }

		/// <summary>Returns the current distance between the two joint bodies.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Distance
		{
			get { return Internal_getDistance(mCachedPtr); }
		}

		/// <summary>
		/// Determines the minimum distance the bodies are allowed to be at, they will get no closer. You must enable min 
		/// distance flag in order for this limit to be applied.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MinDistance
		{
			get { return Internal_getMinDistance(mCachedPtr); }
			set { Internal_setMinDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum distance the bodies are allowed to be at, they will get no further. You must enable max 
		/// distance flag in order for this limit to be applied.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MaxDistance
		{
			get { return Internal_getMaxDistance(mCachedPtr); }
			set { Internal_setMaxDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the error tolerance of the joint at which the joint becomes active. This value slightly extends the lower 
		/// and upper limit.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Tolerance
		{
			get { return Internal_getTolerance(mCachedPtr); }
			set { Internal_setTolerance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a spring that controls how the joint responds when a limit is reached. You must enable the spring  flag on 
		/// the joint in order for this to be recognized.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Spring Spring
		{
			get
			{
				Spring temp;
				Internal_getSpring(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setSpring(mCachedPtr, ref value); }
		}

		/// <summary>Enables or disables a flag that controls joint behaviour.</summary>
		public void SetFlag(DistanceJointFlag flag, bool enabled)
		{
			Internal_setFlag(mCachedPtr, flag, enabled);
		}

		/// <summary>Checks whether a certain joint flag is enabled.</summary>
		public bool HasFlag(DistanceJointFlag flag)
		{
			return Internal_hasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMinDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMinDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMaxDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMaxDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getTolerance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setTolerance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getSpring(IntPtr thisPtr, out Spring __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSpring(IntPtr thisPtr, ref Spring value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFlag(IntPtr thisPtr, DistanceJointFlag flag, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_hasFlag(IntPtr thisPtr, DistanceJointFlag flag);
	}

	/** @} */
}

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

	/// <summary>A joint that maintains an upper or lower (or both) bound on the distance between two bodies.</summary>
	[ShowInInspector]
	public partial class DistanceJoint : Joint
	{
		private DistanceJoint(bool __dummy0) { }
		protected DistanceJoint() { }

		/// <summary>Returns the current distance between the two joint bodies.</summary>
		[NativeWrapper]
		public float Distance
		{
			get { return Internal_GetDistance(mCachedPtr); }
		}

		/// <summary>
		/// Determines the minimum distance the bodies are allowed to be at, they will get no closer. You must enable min 
		/// distance flag in order for this limit to be applied.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MinDistance
		{
			get { return Internal_GetMinDistance(mCachedPtr); }
			set { Internal_SetMinDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum distance the bodies are allowed to be at, they will get no further. You must enable max 
		/// distance flag in order for this limit to be applied.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MaxDistance
		{
			get { return Internal_GetMaxDistance(mCachedPtr); }
			set { Internal_SetMaxDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the error tolerance of the joint at which the joint becomes active. This value slightly extends the lower 
		/// and upper limit.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Tolerance
		{
			get { return Internal_GetTolerance(mCachedPtr); }
			set { Internal_SetTolerance(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a spring that controls how the joint responds when a limit is reached. You must enable the spring flag on 
		/// the joint in order for this to be recognized.
		/// </summary>
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

		/// <summary>Enables or disables a flag that controls joint behaviour.</summary>
		public void SetFlag(DistanceJointFlag flag, bool enabled)
		{
			Internal_SetFlag(mCachedPtr, flag, enabled);
		}

		/// <summary>Checks whether a certain joint flag is enabled.</summary>
		public bool HasFlag(DistanceJointFlag flag)
		{
			return Internal_HasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMinDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMinDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMaxDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMaxDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTolerance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetTolerance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpring(IntPtr thisPtr, ref Spring value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSpring(IntPtr thisPtr, out Spring __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlag(IntPtr thisPtr, DistanceJointFlag flag, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasFlag(IntPtr thisPtr, DistanceJointFlag flag);
	}

	/** @} */
}

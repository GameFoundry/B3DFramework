//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>
	/// A spherical joint removes all translational degrees of freedom but allows all rotational degrees of freedom. 
	/// Essentially this ensures that the anchor points of the two bodies are always coincident. Bodies are allowed to rotate 
	/// around the anchor points, and their rotation can be limited by an elliptical cone.
	/// </summary>
	[ShowInInspector]
	public partial class SphericalJoint : Joint
	{
		private SphericalJoint(bool __dummy0) { }
		protected SphericalJoint() { }

		/// <summary>
		/// Determines the limit of the joint. This clamps the rotation inside an eliptical angular cone. You must enable limit 
		/// flag on the joint in order for this to be recognized.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public LimitConeRange Limit
		{
			get
			{
				LimitConeRange temp;
				Internal_GetLimit(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetLimit(mCachedPtr, ref value); }
		}

		/// <summary>Enables or disables a flag that controls the joint&apos;s behaviour.</summary>
		public void SetFlag(SphericalJointFlag flag, bool isEnabled)
		{
			Internal_SetFlag(mCachedPtr, flag, isEnabled);
		}

		/// <summary>Checks is the specified flag enabled.</summary>
		public bool HasFlag(SphericalJointFlag flag)
		{
			return Internal_HasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLimit(IntPtr thisPtr, ref LimitConeRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetLimit(IntPtr thisPtr, out LimitConeRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlag(IntPtr thisPtr, SphericalJointFlag flag, bool isEnabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_HasFlag(IntPtr thisPtr, SphericalJointFlag flag);
	}

	/** @} */
}

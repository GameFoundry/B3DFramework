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
	/// A spherical joint removes all translational degrees of freedom but allows all rotational degrees of freedom.  
	/// Essentially this ensures that the anchor points of the two bodies are always coincident. Bodies are allowed to rotate 
	/// around the anchor points, and their rotatation can be limited by an elliptical cone.
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
				Internal_getLimit(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setLimit(mCachedPtr, ref value); }
		}

		/// <summary>Enables or disables a flag that controls the joint's behaviour.</summary>
		public void SetFlag(SphericalJointFlag flag, bool enabled)
		{
			Internal_setFlag(mCachedPtr, flag, enabled);
		}

		/// <summary>Checks is the specified flag enabled.</summary>
		public bool HasFlag(SphericalJointFlag flag)
		{
			return Internal_hasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getLimit(IntPtr thisPtr, out LimitConeRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLimit(IntPtr thisPtr, ref LimitConeRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFlag(IntPtr thisPtr, SphericalJointFlag flag, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_hasFlag(IntPtr thisPtr, SphericalJointFlag flag);
	}

	/** @} */
}

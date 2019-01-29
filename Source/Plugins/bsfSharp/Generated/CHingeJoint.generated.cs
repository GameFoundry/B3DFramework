//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>
	/// Hinge joint removes all but a single rotation degree of freedom from its two attached bodies (for example a door  
	/// hinge).
	/// </summary>
	[ShowInInspector]
	public partial class HingeJoint : Joint
	{
		private HingeJoint(bool __dummy0) { }
		protected HingeJoint() { }

		/// <summary>Returns the current angle between the two attached bodes.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Radian Angle
		{
			get
			{
				Radian temp;
				Internal_getAngle(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns the current angular speed of the joint.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Speed
		{
			get { return Internal_getSpeed(mCachedPtr); }
		}

		/// <summary>
		/// Determines the limit of the joint. Limit constrains the motion to the specified angle range. You must enable the 
		/// limit flag on the joint in order for this to be recognized.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public LimitAngularRange Limit
		{
			get
			{
				LimitAngularRange temp;
				Internal_getLimit(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setLimit(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the drive properties of the joint. It drives the joint's angular velocity towards a particular value.  You 
		/// must enable the drive flag on the joint in order for the drive to be active.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public HingeJointDrive Drive
		{
			get
			{
				HingeJointDrive temp;
				Internal_getDrive(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setDrive(mCachedPtr, ref value); }
		}

		/// <summary>Enables or disables a flag that controls joint behaviour.</summary>
		public void SetFlag(HingeJointFlag flag, bool enabled)
		{
			Internal_setFlag(mCachedPtr, flag, enabled);
		}

		/// <summary>Checks is the specified option enabled.</summary>
		public bool HasFlag(HingeJointFlag flag)
		{
			return Internal_hasFlag(mCachedPtr, flag);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getAngle(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getSpeed(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getLimit(IntPtr thisPtr, out LimitAngularRange __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLimit(IntPtr thisPtr, ref LimitAngularRange limit);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getDrive(IntPtr thisPtr, out HingeJointDrive __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setDrive(IntPtr thisPtr, ref HingeJointDrive drive);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFlag(IntPtr thisPtr, HingeJointFlag flag, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_hasFlag(IntPtr thisPtr, HingeJointFlag flag);
	}

	/** @} */
}

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

	/// <summary>Properties of a drive that drives the joint&apos;s angular velocity towards a paricular value.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct HingeJointDrive
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static HingeJointDrive Default()
		{
			HingeJointDrive value = new HingeJointDrive();
			value.Speed = 0f;
			value.ForceLimit = 3.40282347E+38f;
			value.GearRatio = 1f;
			value.FreeSpin = false;

			return value;
		}

		/// <summary>Target speed of the joint.</summary>
		public float Speed;
		/// <summary>Maximum torque the drive is allowed to apply .</summary>
		public float ForceLimit;
		/// <summary>Scales the velocity of the first body, and its response to drive torque is scaled down.</summary>
		public float GearRatio;
		/// <summary>
		/// If the joint is moving faster than the drive&apos;s target speed, the drive will try to break. If you don&apos;t want 
		/// the breaking to happen set this to true.
		/// </summary>
		public bool FreeSpin;
	}

	/** @} */
}

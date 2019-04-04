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

	/// <summary>Properties of a drive that drives the joint&apos;s angular velocity towards a paricular value.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct HingeJointDrive
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static HingeJointDrive Default()
		{
			HingeJointDrive value = new HingeJointDrive();
			value.speed = 0f;
			value.forceLimit = 3.40282347E+38f;
			value.gearRatio = 1f;
			value.freeSpin = false;

			return value;
		}

		/// <summary>Target speed of the joint.</summary>
		public float speed;
		/// <summary>Maximum torque the drive is allowed to apply .</summary>
		public float forceLimit;
		/// <summary>Scales the velocity of the first body, and its response to drive torque is scaled down.</summary>
		public float gearRatio;
		/// <summary>
		/// If the joint is moving faster than the drive&apos;s target speed, the drive will try to break. If you don&apos;t want 
		/// the breaking to happen set this to true.
		/// </summary>
		public bool freeSpin;
	}

	/** @} */
}

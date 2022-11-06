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

	/// <summary>
	/// Specifies parameters for a drive that will attempt to move the joint bodies to the specified drive position and 
	/// velocity.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct D6JointDrive
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static D6JointDrive Default()
		{
			D6JointDrive value = new D6JointDrive();
			value.Stiffness = 0f;
			value.Damping = 0f;
			value.ForceLimit = 3.40282347E+38f;
			value.Acceleration = false;

			return value;
		}

		/// <summary>Spring strength. Force proportional to the position error.</summary>
		public float Stiffness;
		/// <summary>Damping strength. Force propertional to the velocity error.</summary>
		public float Damping;
		/// <summary>Maximum force the drive can apply.</summary>
		public float ForceLimit;
		/// <summary>
		/// If true the drive will generate acceleration instead of forces. Acceleration drives are easier to tune as they 
		/// account for the masses of the actors to which the joint is attached.
		/// </summary>
		public bool Acceleration;
	}

	/** @} */
}

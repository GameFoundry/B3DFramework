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

	/// <summary>Type of drives that can be used for moving or rotating bodies attached to the joint.</summary>
	public enum D6JointDriveType
	{
		/// <summary>Rotation around all three axes at once. Only active with D6JointAngularDriveMode::SLERP.</summary>
		SLERP = 6,
		/// <summary>Linear movement on the X axis.</summary>
		X = 0,
		/// <summary>Rotation around all three axes at once. Only active with D6JointAngularDriveMode::SLERP.</summary>
		Count = 7,
		/// <summary>Linear movement on the Y axis.</summary>
		Y = 1,
		/// <summary>Linear movement on the Z axis.</summary>
		Z = 2,
		/// <summary>Rotation around the Y axis. Only active with D6JointAngularDriveMode::SwingTwist.</summary>
		SwingY = 4,
		/// <summary>Rotation around the X axis. Only active with D6JointAngularDriveMode::SwingTwist.</summary>
		Twist = 3,
		/// <summary>Rotation around the Z axis. Only active with D6JointAngularDriveMode::SwingTwist.</summary>
		SwingZ = 5
	}

	/** @} */
}

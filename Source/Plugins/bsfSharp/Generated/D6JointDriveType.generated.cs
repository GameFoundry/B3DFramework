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

	/// <summary>Type of drives that can be used for moving or rotating bodies attached to the joint.</summary>
	public enum D6JointDriveType
	{
		/// <summary>Linear movement on the X axis using the linear drive model.</summary>
		X = 0,
		/// <summary>Linear movement on the Y axis using the linear drive model.</summary>
		Y = 1,
		/// <summary>Linear movement on the Z axis using the linear drive model.</summary>
		Z = 2,
		/// <summary>
		/// Rotation around the Y axis using the twist/swing angular drive model. Should not be used together with  SLERP mode.
		/// </summary>
		Swing = 3,
		/// <summary>
		/// Rotation around the Z axis using the twist/swing angular drive model. Should not be used together with  SLERP mode.
		/// </summary>
		Twist = 4,
		/// <summary>
		/// Rotation using spherical linear interpolation. Uses the SLERP angular drive mode which performs rotation by 
		/// interpolating the quaternion values directly over the shortest path (applies to all three axes, which they all must 
		/// be unlocked).
		/// </summary>
		SLERP = 5,
		/// <summary>
		/// Rotation using spherical linear interpolation. Uses the SLERP angular drive mode which performs rotation by 
		/// interpolating the quaternion values directly over the shortest path (applies to all three axes, which they all must 
		/// be unlocked).
		/// </summary>
		Count = 6
	}

	/** @} */
}

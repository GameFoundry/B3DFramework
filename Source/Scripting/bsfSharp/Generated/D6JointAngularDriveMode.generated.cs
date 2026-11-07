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

	/// <summary>Determines how the angular drives of a D6 joint rotate the bodies towards the drive target.</summary>
	public enum D6JointAngularDriveMode
	{
		/// <summary>Each angular axis is driven separately, using the Twist, SwingY and SwingZ drives.</summary>
		SwingTwist = 0,
		/// <summary>
		/// All angular axes are driven together along the shortest rotation, using the SLERP drive. Has no effect unless the 
		/// Twist, SwingY and SwingZ axes are all unlocked.
		/// </summary>
		SLERP = 1
	}

	/** @} */
}

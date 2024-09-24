//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Input
	 *  @{
	 */

	/// <summary>Common input axis types.</summary>
	public enum InputAxis
	{
		/// <summary>Gamepad left trigger. Provides normalized ([-1, 1] range) absolute position.</summary>
		LeftTrigger = 7,
		Count = 9,
		/// <summary>Mouse axis X. Provides unnormalized relative movement.</summary>
		MouseX = 0,
		/// <summary>Mouse axis Y. Provides unnormalized relative movement.</summary>
		MouseY = 1,
		/// <summary>Mouse wheel/scroll axis. Provides unnormalized relative movement.</summary>
		MouseZ = 2,
		/// <summary>Gamepad right stick Y. Provides normalized ([-1, 1] range) absolute position.</summary>
		RightStickY = 6,
		/// <summary>Gamepad left stick X. Provides normalized ([-1, 1] range) absolute position.</summary>
		LeftStickX = 3,
		/// <summary>Gamepad left stick Y. Provides normalized ([-1, 1] range) absolute position.</summary>
		LeftStickY = 4,
		/// <summary>Gamepad right stick X. Provides normalized ([-1, 1] range) absolute position.</summary>
		RightStickX = 5,
		/// <summary>Gamepad right trigger. Provides normalized ([-1, 1] range) absolute position.</summary>
		RightTrigger = 8
	}

	/** @} */
}

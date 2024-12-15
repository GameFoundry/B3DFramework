//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>Contains options that change GUIElement behaviour.</summary>
	public enum GUIElementOption
	{
		/// <summary>
		/// Enable this option if you want pointer events to pass through this element by default. This will allow elements 
		/// underneath this element to receive pointer events.
		/// </summary>
		ClickThrough = 1,
		/// <summary>
		/// Enable this option if the element accepts keyboard/gamepad input focus. This will allow the element to be navigated 
		/// to using keys/buttons.
		/// </summary>
		AcceptsKeyFocus = 2,
		/// <summary>Pointer events on the GUI element will be ignored.</summary>
		IgnorePointerEvents = 4
	}

	/** @} */
}

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

	/// <summary>Flags that control how does a slider handle behave.</summary>
	public enum GUISliderHandleFlag
	{
		/// <summary>Determines should the slider handle provide additional side-handles that allow it to be resized.</summary>
		Resizeable = 8,
		/// <summary>Slider handle will move horizontally. Cannot be used with the Vertical option.</summary>
		Horizontal = 1,
		/// <summary>Slider handle will move vertically. Cannot be used with the Horizontal option.</summary>
		Vertical = 2,
		/// <summary>
		/// If enabled, clicking on a specific slider position will cause the handle to jump to that position. If false the 
		/// handle will only slightly move in that direction.
		/// </summary>
		JumpOnClick = 4
	}

	/** @} */
}

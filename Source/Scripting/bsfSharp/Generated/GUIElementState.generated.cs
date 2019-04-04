//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>Type of GUI element states.</summary>
	public enum GUIElementState
	{
		/// <summary>Same as Hover, if the element is also in the &quot;on&quot; state.</summary>
		HoverOn = 258,
		/// <summary>Normal state when element is not being iteracted with.</summary>
		Normal = 1,
		/// <summary>Same as Normal, if the element is also in the &quot;on&quot; state.</summary>
		NormalOn = 257,
		/// <summary>State when pointer is hovering over the element.</summary>
		Hover = 2,
		/// <summary>State when element is being clicked.</summary>
		Active = 4,
		/// <summary>State when the element has input focus and pointer is hovering over the element.</summary>
		FocusedHover = 16,
		/// <summary>State when the element has input focus and pointer is not hovering over the element.</summary>
		Focused = 8,
		/// <summary>Same as Active, if the element is also in the &quot;on&quot; state.</summary>
		ActiveOn = 260,
		/// <summary>Same as Focused, if the element is also in the &quot;on&quot; state.</summary>
		FocusedOn = 264,
		/// <summary>Same as FocusedHover, if the element is also in the &quot;on&quot; state.</summary>
		FocusedHoverOn = 272
	}

	/** @} */
}

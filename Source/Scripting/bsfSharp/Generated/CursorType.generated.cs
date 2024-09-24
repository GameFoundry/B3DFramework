//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Platform
	 *  @{
	 */

	/// <summary>Available cursor types.</summary>
	public enum CursorType
	{
		Arrow = 0,
		ArrowDrag = 1,
		Wait = 3,
		SizeNWSE = 7,
		SizeNS = 6,
		Deny = 9,
		ArrowLeftRight = 2,
		SizeNESW = 5,
		IBeam = 4,
		SizeWE = 8,
		Count = 10
	}

	/** @} */
}

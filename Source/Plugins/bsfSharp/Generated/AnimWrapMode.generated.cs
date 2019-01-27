//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>Determines how an animation clip behaves when it reaches the end.</summary>
	public enum AnimWrapMode
	{
		/// <summary>Loop around to the beginning/end when the last/first frame is reached.</summary>
		Loop = 0,
		/// <summary>Clamp to end/beginning, keeping the last/first frame active.</summary>
		Clamp = 1
	}

	/** @} */
}

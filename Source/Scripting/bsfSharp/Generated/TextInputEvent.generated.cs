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

	/// <summary>
	/// Event that gets sent out when user inputs some text. These events may be preceeded by normal button events if user is 
	/// typing on a keyboard.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct TextInputEvent
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static TextInputEvent Default()
		{
			TextInputEvent value = new TextInputEvent();
			value.TextChar = 0;
			value.IsUsed = false;

			return value;
		}

		/// <summary>Character the that was input.</summary>
		public int TextChar;
		/// <summary>This will be set to true if some previous event receiver has marked the event as used.</summary>
		public bool IsUsed;
	}

	/** @} */
}

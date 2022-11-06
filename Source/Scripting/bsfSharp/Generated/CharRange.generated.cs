//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Text
	 *  @{
	 */

	/// <summary>Represents a range of character code.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct CharRange
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static CharRange Default()
		{
			CharRange value = new CharRange();
			value.Start = 0;
			value.End = 0;

			return value;
		}

		public CharRange(int start, int end)
		{
			this.Start = start;
			this.End = end;
		}

		public int Start;
		public int End;
	}

	/** @} */
#endif
}

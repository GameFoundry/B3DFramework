//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>Rectangle represented in the form of offsets from some parent rectangle.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct RectOffset
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static RectOffset Default()
		{
			RectOffset value = new RectOffset();
			value.Left = 0;
			value.Right = 0;
			value.Top = 0;
			value.Bottom = 0;

			return value;
		}

		public RectOffset(int left, int right, int top, int bottom)
		{
			this.Left = left;
			this.Right = right;
			this.Top = top;
			this.Bottom = bottom;
		}

		public int Left;
		public int Right;
		public int Top;
		public int Bottom;
	}

	/** @} */
}

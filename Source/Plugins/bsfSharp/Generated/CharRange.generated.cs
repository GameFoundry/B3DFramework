//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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
			value.start = 0;
			value.end = 0;

			return value;
		}

		public CharRange(int start, int end)
		{
			this.start = start;
			this.end = end;
		}

		public int start;
		public int end;
	}

	/** @} */
#endif
}

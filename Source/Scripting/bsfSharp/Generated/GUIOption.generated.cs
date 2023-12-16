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

	/// <summary>
	/// Controls GUI element layout options, possibly by overriding the default options specified in GUI element style. These 
	/// options control GUI element placement and size in a GUI layout.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct GUIOption
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static GUIOption Default()
		{
			GUIOption value = new GUIOption();
			value.min = 0;
			value.max = 0;
			value.type = GUIOptionType.FixedWidth;

			return value;
		}

		public int min;
		public int max;
		public GUIOptionType type;
	}

	/** @} */
}

//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Image
	 *  @{
	 */

	/// <summary>Single key in a ColorGradient.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ColorGradientKey
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ColorGradientKey Default()
		{
			ColorGradientKey value = new ColorGradientKey();
			value.Color = Color.Default();
			value.Time = 0f;

			return value;
		}

		public ColorGradientKey(Color color, float time)
		{
			this.Color = color;
			this.Time = time;
		}

		public Color Color;
		public float Time;
	}

	/** @} */
}

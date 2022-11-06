//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/// <summary>Represents a single potential value of a shader variation parameter and optionally its name.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ShaderVariationParamValue
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ShaderVariationParamValue Default()
		{
			ShaderVariationParamValue value = new ShaderVariationParamValue();
			value.Name = "";
			value.Value = 0;

			return value;
		}

		/// <summary>Optional human-readable name describing what this particular value represents.</summary>
		public string Name;
		/// <summary>Integer value of the parameter.</summary>
		public int Value;
	}

	/** @} */
}

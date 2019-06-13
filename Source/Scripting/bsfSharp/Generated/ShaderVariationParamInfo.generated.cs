//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/// <summary>Represents a single shader variation parameter and a set of all possible values.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ShaderVariationParamInfo
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ShaderVariationParamInfo Default()
		{
			ShaderVariationParamInfo value = new ShaderVariationParamInfo();
			value.name = "";
			value.identifier = "";
			value.isInternal = true;
			value.values = null;

			return value;
		}

		/// <summary>Optional human-readable name describing the variation parameter.</summary>
		public string name;
		/// <summary>BSL identifier for the parameter.</summary>
		public string identifier;
		/// <summary>
		/// True if the parameter is for internal use by the renderer, and false if its intended to be set by the user.
		/// </summary>
		public bool isInternal;
		/// <summary>A list of potential values this parameter can take on.</summary>
		public ShaderVariationParamValue[] values;
	}

	/** @} */
}

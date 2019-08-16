//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Flags used to further describe a shader parameter.</summary>
	public enum ShaderParameterFlag
	{
		None = 0,
		/// <summary>Parameter is for internal use by the renderer and isn&apos;t expected to be set by the user.</summary>
		Internal = 1,
		/// <summary>Parameter should not be displayed in the editor inspector.</summary>
		HideInInspector = 2,
		/// <summary>Allows the color parameter to be edited using the HDR color picker.</summary>
		HDR = 4
	}

	/** @} */
}

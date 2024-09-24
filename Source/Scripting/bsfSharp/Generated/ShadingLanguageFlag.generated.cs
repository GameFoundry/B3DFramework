//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>Supported types of low-level shading languages.</summary>
	public enum ShadingLanguageFlags
	{
		/// <summary>Variant of GLSL used for Vulkan.</summary>
		VKSL = 4,
		/// <summary>OpenGL shading language.</summary>
		GLSL = 2,
		Unknown = 0,
		/// <summary>High level shading language used by DirectX backend.</summary>
		HLSL = 1,
		/// <summary>Metal shading language.</summary>
		MSL = 8,
		/// <summary>Helper entry that includes all languages.</summary>
		All = 15,
		/// <summary>Metal shading language.</summary>
		Count = 4
	}

	/** @} */
#endif
}

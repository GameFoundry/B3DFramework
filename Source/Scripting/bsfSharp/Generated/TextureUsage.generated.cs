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

	/// <summary>Flags that describe how is a texture used.</summary>
	public enum TextureUsage
	{
		/// <summary>A regular texture that is not often or ever updated from the CPU.</summary>
		Default = 1,
		/// <summary>A regular texture that is often updated by the CPU.</summary>
		Dynamic = 2,
		/// <summary>Texture that can be rendered to by the GPU.</summary>
		Render = 512,
		/// <summary>Texture used as a depth/stencil buffer by the GPU.</summary>
		DepthStencil = 1024,
		/// <summary>Texture that allows load/store operations from the GPU program.</summary>
		LoadStore = 2048,
		/// <summary>
		/// All mesh data will also be cached in CPU memory, making it available for fast read access from the CPU.
		/// </summary>
		CPUCached = 4096,
		/// <summary>Allows the CPU to directly read the texture data buffers from the GPU.</summary>
		CPUReadable = 8192,
		/// <summary>
		/// Allows you to retrieve views of the texture using a format different from one specified on creation.
		/// </summary>
		MutableFormat = 16384
	}

	/** @} */
}

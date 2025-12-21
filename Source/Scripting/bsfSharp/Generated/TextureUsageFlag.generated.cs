//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Flags that describe how a texture is used.</summary>
	public enum TextureUsageFlag
	{
		/// <summary>Allows retrieving views of the texture using a different format than specified on creation.</summary>
		MutableFormat = 16384,
		/// <summary>Texture used as a depth/stencil buffer by the GPU. Must be combined with StoreOnGPU flag.</summary>
		DepthStencil = 1024,
		/// <summary>
		/// Places the texture into CPU memory accessible to the GPU. This means the texture is faster to update from the CPU, 
		/// but it&apos;s slower to access by the GPU. Not allowed for GPU-writeable textures (render target, depth stencil or 
		/// unordered access).
		/// </summary>
		Dynamic = 2,
		/// <summary>Default setting suitable for majority of textures.</summary>
		Default = 1,
		/// <summary>Texture that can be rendered to by the GPU. Must be combined with StoreOnGPU flag.</summary>
		Render = 512,
		/// <summary>
		/// Ensures that the GPU can perform unordered write operations on the texture. Generally used for textures in compute 
		/// operations. Must be combined with StoreOnGPU flag.
		/// </summary>
		LoadStore = 2048,
		/// <summary>
		/// All texture data will also be cached in CPU memory for fast read access from the CPU. Only relevant for main thread 
		/// textures, ignored for render thread textures.
		/// </summary>
		CPUCached = 4096
	}

	/** @} */
}

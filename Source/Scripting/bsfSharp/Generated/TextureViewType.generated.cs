//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Type of view through which GPU programs access a texture. See TextureSurface::ViewType.</summary>
	public enum TextureViewType
	{
		/// <summary>
		/// Follows from the texture: textures with more than one array slice are viewed as arrays, and cube maps as cubes, or as 
		/// cube arrays if they have more than one. Storage views of cube maps are 2D arrays, as cube views cannot be written to.
		/// </summary>
		Default = 0,
		/// <summary>Single slice of a one dimensional texture. Can only view the first slice.</summary>
		Texture1D = 1,
		/// <summary>Range of slices of a one dimensional texture.</summary>
		Texture1DArray = 2,
		/// <summary>Single slice of a two dimensional texture or a cube map. Can only view the first slice.</summary>
		Texture2D = 3,
		/// <summary>Range of slices of a two dimensional texture, or faces of a cube map.</summary>
		Texture2DArray = 4,
		/// <summary>Three dimensional texture.</summary>
		Texture3D = 5,
		/// <summary>First cube of a cube map. Not supported by storage views.</summary>
		TextureCube = 6,
		/// <summary>Range of whole cubes of a cube map. Not supported by storage views.</summary>
		TextureCubeArray = 7
	}

	/** @} */
}

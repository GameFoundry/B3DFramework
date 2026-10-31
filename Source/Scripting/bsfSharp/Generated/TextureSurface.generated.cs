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

	/// <summary>References a subset of surfaces within a texture.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct TextureSurface
	{
		public TextureSurface(int mipLevel = 0, int mipLevelCount = 1, int face = 0, int faceCount = 1, TextureViewType viewType = TextureViewType.Default)
		{
			this.MipLevel = mipLevel;
			this.MipLevelCount = mipLevelCount;
			this.Face = face;
			this.FaceCount = faceCount;
			this.ViewType = viewType;
		}

		/// <summary>First mip level to reference.</summary>
		public int MipLevel;
		/// <summary>Number of mip levels to reference. Must be greater than zero.</summary>
		public int MipLevelCount;
		/// <summary>
		/// First face to reference. Face can represent a single cubemap face, or a single array entry in a texture array. If 
		/// cubemaps are laid out in a texture array then every six sequential faces represent a single array entry.
		/// </summary>
		public int Face;
		/// <summary>Number of faces to reference, if the texture has more than one.</summary>
		public int FaceCount;
		/// <summary>
		/// Type of view GPU programs access the surface through. Must match the type of the parameter the surface is bound to. See 
		/// TextureProperties::ValidateSurface() for the surfaces each type can view.
		/// </summary>
		public TextureViewType ViewType;
	}

	/** @} */
}

//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>References a subset of surfaces within a texture.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct TextureSurface
	{
		public TextureSurface(int mipLevel = 0, int mipLevelCount = 1, int face = 0, int faceCount = 1, bool isBoundAs2DArray = false)
		{
			this.MipLevel = mipLevel;
			this.MipLevelCount = mipLevelCount;
			this.Face = face;
			this.FaceCount = faceCount;
			this.IsBoundAs2DArray = isBoundAs2DArray;
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
		/// <summary>Forces a cubemap or a 3D texture to be bound as a 2D texture array.</summary>
		public bool IsBoundAs2DArray;
	}

	/** @} */
}

//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>References a subset of surfaces within a texture.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct TextureSurface
	{
		public TextureSurface(int mipLevel = 0, int numMipLevels = 1, int face = 0, int numFaces = 1)
		{
			this.mipLevel = mipLevel;
			this.numMipLevels = numMipLevels;
			this.face = face;
			this.numFaces = numFaces;
		}

		/// <summary>First mip level to reference.</summary>
		public int mipLevel;
		/// <summary>Number of mip levels to reference. Must be greater than zero.</summary>
		public int numMipLevels;
		/// <summary>
		/// First face to reference. Face can represent a single cubemap face, or a single array entry in a  texture array. If 
		/// cubemaps are laid out in a texture array then every six sequential faces represent a single array entry.
		/// </summary>
		public int face;
		/// <summary>Number of faces to reference, if the texture has more than one.</summary>
		public int numFaces;
	}

	/** @} */
}

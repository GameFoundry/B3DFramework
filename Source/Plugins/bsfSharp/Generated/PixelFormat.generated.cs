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

	/// <summary>Pixel formats usable by images, textures and render surfaces.</summary>
	public enum PixelFormat
	{
		/// <summary>8-bit 1-channel pixel format, unsigned normalized.</summary>
		R8 = 1,
		/// <summary>8-bit 2-channel pixel format, unsigned normalized.</summary>
		RG8 = 2,
		/// <summary>8-bit 3-channel pixel format, unsigned normalized.</summary>
		RGB8 = 3,
		/// <summary>8-bit 3-channel pixel format, unsigned normalized.</summary>
		BGR8 = 4,
		/// <summary>DXT3/BC2 format containing RGB with explicit alpha. 8 bits per pixel.</summary>
		BC2 = 15,
		/// <summary>8-bit 4-channel pixel format, unsigned normalized.</summary>
		BGRA8 = 7,
		/// <summary>
		/// DXT5/BC2 format containing RGB with explicit alpha. 8 bits per pixel. Better alpha gradients than BC2.
		/// </summary>
		BC3 = 16,
		/// <summary>8-bit 4-channel pixel format, unsigned normalized.</summary>
		RGBA8 = 8,
		/// <summary>DXT1/BC1 format containing opaque RGB or 1-bit alpha RGB. 4 bits per pixel.</summary>
		BC1 = 13,
		/// <summary>One channel compressed format. 4 bits per pixel.</summary>
		BC4 = 17,
		/// <summary>Two channel compressed format. 8 bits per pixel.</summary>
		BC5 = 18,
		/// <summary>Format storing RGB in half (16-bit) floating point format usable for HDR. 8 bits per pixel.</summary>
		BC6H = 19,
		/// <summary>
		/// Format storing RGB with optional alpha channel. Similar to BC1/BC2/BC3 formats but with higher quality and  higher 
		/// decompress overhead. 8 bits per pixel.
		/// </summary>
		BC7 = 20,
		/// <summary>16-bit 1-channel pixel format, signed float.</summary>
		R16F = 21,
		/// <summary>16-bit 2-channel pixel format, signed float.</summary>
		RG16F = 22,
		/// <summary>16-bit 4-channel pixel format, signed float.</summary>
		RGBA16F = 24,
		/// <summary>32-bit 1-channel pixel format, signed float.</summary>
		R32F = 25,
		/// <summary>32-bit 2-channel pixel format, signed float.</summary>
		RG32F = 26,
		/// <summary>32-bit 3-channel pixel format, signed float.</summary>
		RGB32F = 27,
		/// <summary>32-bit 4-channel pixel format, signed float.</summary>
		RGBA32F = 28,
		/// <summary>Depth stencil format, 32bit depth, 8bit stencil + 24 unused. Depth stored as signed float.</summary>
		D32_S8X24 = 29,
		/// <summary>Depth stencil fomrat, 24bit depth + 8bit stencil. Depth stored as unsigned normalized.</summary>
		D24S8 = 30,
		/// <summary>Depth format, 32bits. Signed float.</summary>
		D32 = 31,
		/// <summary>Depth format, 16bits. Unsigned normalized.</summary>
		D16 = 32,
		/// <summary>8-bit 1-channel pixel format, signed integer.</summary>
		R8I = 35,
		/// <summary>8-bit 2-channel pixel format, signed integer.</summary>
		RG8I = 36,
		/// <summary>8-bit 4-channel pixel format, signed integer.</summary>
		RGBA8I = 37,
		/// <summary>8-bit 1-channel pixel format, unsigned integer.</summary>
		R8U = 38,
		/// <summary>8-bit 2-channel pixel format, unsigned integer.</summary>
		RG8U = 39,
		/// <summary>8-bit 4-channel pixel format, unsigned integer.</summary>
		RGBA8U = 40,
		/// <summary>8-bit 1-channel pixel format, signed normalized.</summary>
		R8S = 41,
		/// <summary>8-bit 2-channel pixel format, signed normalized.</summary>
		RG8S = 42,
		/// <summary>8-bit 4-channel pixel format, signed normalized.</summary>
		RGBA8S = 43,
		/// <summary>16-bit 1-channel pixel format, signed integer.</summary>
		R16I = 44,
		/// <summary>16-bit 2-channel pixel format, signed integer.</summary>
		RG16I = 45,
		/// <summary>16-bit 4-channel pixel format, signed integer.</summary>
		RGBA16I = 46,
		/// <summary>16-bit 1-channel pixel format, unsigned integer.</summary>
		R16U = 47,
		/// <summary>16-bit 2-channel pixel format, unsigned integer.</summary>
		RG16U = 48,
		/// <summary>16-bit 4-channel pixel format, unsigned integer.</summary>
		RGBA16U = 49,
		/// <summary>32-bit 1-channel pixel format, signed integer.</summary>
		R32I = 50,
		/// <summary>32-bit 2-channel pixel format, signed integer.</summary>
		RG32I = 51,
		/// <summary>32-bit 3-channel pixel format, signed integer.</summary>
		RGB32I = 52,
		/// <summary>32-bit 4-channel pixel format, signed integer.</summary>
		RGBA32I = 53,
		/// <summary>32-bit 1-channel pixel format, unsigned integer.</summary>
		R32U = 54,
		/// <summary>32-bit 2-channel pixel format, unsigned integer.</summary>
		RG32U = 55,
		/// <summary>32-bit 3-channel pixel format, unsigned integer.</summary>
		RGB32U = 56,
		/// <summary>32-bit 4-channel pixel format, unsigned integer.</summary>
		RGBA32U = 57,
		/// <summary>16-bit 1-channel pixel format, signed normalized.</summary>
		R16S = 58,
		/// <summary>16-bit 2-channel pixel format, signed normalized.</summary>
		RG16S = 59,
		/// <summary>16-bit 4-channel pixel format, signed normalized.</summary>
		RGBA16S = 60,
		/// <summary>16-bit 1-channel pixel format, unsigned normalized.</summary>
		R16 = 61,
		/// <summary>16-bit 2-channel pixel format, unsigned normalized.</summary>
		RG16 = 62,
		/// <summary>16-bit 3-channel pixel format, unsigned normalized.</summary>
		RGB16 = 63,
		/// <summary>16-bit 4-channel pixel format, unsigned normalized.</summary>
		RGBA16 = 64
	}

	/** @} */
}

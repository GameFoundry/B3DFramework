//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DPixelData.h"

namespace b3d
{
	struct CompressionOptions;

	/**
	 * GPU-accelerated block compression. Encodes uncompressed pixel data into a block-compressed format using a compute
	 * shader. Supports BC1, BC3, BC4, BC5, BC6H and BC7.
	 */
	class B3D_EXPORT GpuTextureCompressor
	{
	public:
		/** Returns true if the GPU compressor supports encoding to the provided block-compressed format. */
		static bool IsFormatSupported(PixelFormat format);

		/**
		 * Encodes @p source (uncompressed) into @p destination (block-compressed) on the GPU. Blocks until the GPU has
		 * finished and the result has been read back to CPU memory. Returns false if compression could not be performed
		 * (e.g. unsupported format or no active GPU device).
		 */
		static bool Compress(const PixelData& source, PixelData& destination, const CompressionOptions& options);
	};

	/**
	 * CPU reference decoders for block-compressed formats. These mirror the hardware BCn decoder bit-for-bit and exist
	 * primarily for verification (e.g. measuring encoder PSNR in tests), not for runtime decompression.
	 */
	class B3D_EXPORT TextureCompressionUtility
	{
	public:
		/**
		 * Decodes a BC1 color block (8 bytes) into 16 RGBA quads (@p outRGBA, row-major). When @p fourColorOnly is true the
		 * 3-colour/punch-through mode is never used (as for the BC2/BC3 color sub-block).
		 */
		static void DecodeBC1(const u8* block, bool fourColorOnly, u8 outRGBA[64]);

		/** Decodes a BC4 block (8 bytes) into 16 single-channel values (@p out). */
		static void DecodeBC4(const u8* block, u8 out[16]);

		/** Decodes a BC7 block (16 bytes) into 16 RGBA quads (@p outRGBA, row-major). */
		static void DecodeBC7(const u8* block, u8 outRGBA[64]);

		/** Decodes a BC6H block in the unsigned (UF16) variant (16 bytes) into 16 RGB float triplets (@p out). */
		static void DecodeBC6H_UF16(const u8* block, float out[16][3]);
	};
} // namespace b3d

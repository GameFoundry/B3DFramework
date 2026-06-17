//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DPixelData.h"
#include "Threading/B3DAsyncOp.h"

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
		 * Encodes @p source (uncompressed) into @p destination (block-compressed) on the GPU. The returned operation
		 * completes with @p destination (now holding the packed blocks) once the GPU has finished and the result has been
		 * read back, or with null if compression could not be performed (e.g. unsupported format or no active GPU device).
		 * Both surfaces are held by shared pointer for the duration of the operation, so the caller need not keep them alive
		 * itself. The render thread is never blocked on the GPU - the read-back happens in a command-buffer completion
		 * callback.
		 *
		 * Do not block on the returned operation from the render thread, since that thread owns (and drives) the callback.
		 */
		static TAsyncOp<TShared<PixelData>> Compress(const TShared<PixelData>& source, const TShared<PixelData>& destination, const CompressionOptions& options);
	};
} // namespace b3d

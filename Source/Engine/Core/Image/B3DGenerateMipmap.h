//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DPixelData.h"

namespace b3d
{
	struct MipMapGenOptions;

	/**
	 * GPU-accelerated mip-map chain generation. Produces the full mip chain for a 2D surface using a compute shader that
	 * downsamples each level from the previous one.
	 */
	class B3D_EXPORT GpuGenerateMipmap
	{
	public:
		/**
		 * Generates the full mip chain for @p source (including mip 0) on the GPU, appending each level - in @p source's
		 * pixel format - to @p output. Blocks until the GPU has finished and every level has been read back to CPU memory.
		 * Returns false if generation could not be performed (e.g. no active GPU device or the shader is unavailable), in
		 * which case @p output is left untouched and the caller may fall back to a CPU path.
		 */
		static bool Generate(const TShared<PixelData>& source, const MipMapGenOptions& options, Vector<TShared<PixelData>>& output);
	};
} // namespace b3d

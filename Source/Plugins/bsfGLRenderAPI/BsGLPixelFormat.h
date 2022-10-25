//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Image/BsPixelUtil.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Converts pixel formats to OpenGL formats */
		class GLPixelUtil
		{
		public:
			/**	Finds the closest pixel format that OpenGL supports. */
			static PixelFormat GetClosestSupportedPf(PixelFormat format, TextureType texType, int usage);

			/**	Returns matching OpenGL base pixel format type if one is found, zero otherwise. */
			static GLenum GetGlOriginFormat(PixelFormat format);

			/**	Returns OpenGL data type used in the provided format. */
			static GLenum GetGlOriginDataType(PixelFormat format);

			/**
			 * Returns matching OpenGL internal pixel format type if one is found, zero otherwise. Optionally returns an SRGB
			 * format if @p hwGamma is specified and such format exists.
			 */
			static GLenum GetGlInternalFormat(PixelFormat format, bool hwGamma = false);

			/** Returns an OpenGL type that should be used for creating a buffer for the specified depth/stencil format. */
			static GLenum GetDepthStencilTypeFromPf(PixelFormat format);

			/** Returns an OpenGL format that should be used for creating a buffer for the specified depth/stencil format. */
			static GLenum GetDepthStencilFormatFromPf(PixelFormat format);

			/** Converts engine GPU buffer format to OpenGL GPU buffer format. */
			static GLenum GetBufferFormat(GpuBufferFormat format);
		};

		/** @} */
	} // namespace ct
} // namespace bs

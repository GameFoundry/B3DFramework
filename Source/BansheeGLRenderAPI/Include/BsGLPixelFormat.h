//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsPixelUtil.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Converts pixel formats to OpenGL formats */
	class BS_RSGL_EXPORT GLPixelUtil
	{
	public:
		/**	Finds the closest pixel format that OpenGL supports. */
		static PixelFormat getClosestSupportedPF(PixelFormat format, TextureType texType, int usage);

		/**	Returns matching OpenGL base pixel format type if one is found, zero otherwise. */
		static GLenum getGLOriginFormat(PixelFormat mFormat);
	
		/**	Returns OpenGL data type used in the provided format. */
		static GLenum getGLOriginDataType(PixelFormat mFormat);
        
		/**
		 * Returns matching OpenGL internal pixel format type if one is found, zero otherwise. Optionally returns an SRGB
		 * format if @p hwGamma is specified and such format exists.
		 */
		static GLenum getGLInternalFormat(PixelFormat mFormat, bool hwGamma = false);
	
		/** Returns an OpenGL type that should be used for creating a buffer for the specified depth/stencil format. */
		static GLenum getDepthStencilTypeFromPF(PixelFormat mFormat);

		/** Returns an OpenGL format that should be used for creating a buffer for the specified depth/stencil format. */
		static GLenum getDepthStencilFormatFromPF(PixelFormat mFormat);

		/** Converts engine GPU buffer format to OpenGL GPU buffer format. */
		static GLenum getBufferFormat(GpuBufferFormat format);

	};

	/** @} */
}}
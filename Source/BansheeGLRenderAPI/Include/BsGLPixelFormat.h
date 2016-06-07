//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsPixelUtil.h"

namespace BansheeEngine 
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Converts pixel formats to OpenGL formats */
	class BS_RSGL_EXPORT GLPixelUtil
	{
	public:
		/**	Returns matching OpenGL base pixel format type if one is found, zero otherwise. */
		static GLenum getGLOriginFormat(PixelFormat mFormat);
	
		/**	Returns OpenGL data type used in the provided format. */
		static GLenum getGLOriginDataType(PixelFormat mFormat);
        
		/**
		 * Returns matching OpenGL internal pixel format type if one is found, zero otherwise. Optionally returns an SRGB
		 * format if @p hwGamma is specified and such format exists.
		 */
		static GLenum getGLInternalFormat(PixelFormat mFormat, bool hwGamma = false);
	
		/**
		 * Returns matching OpenGL internal pixel format type if one is found, otherwise it returns the closest matching
		 * format. Optionally returns an SRGB format if @p hwGamma is specified and such format exists.
		 */
		static GLenum getClosestGLInternalFormat(PixelFormat mFormat, bool hwGamma = false);
		
		/** Returns an OpenGL type that should be used for creating a buffer for the specified depth/stencil format. */
		static GLenum getDepthStencilTypeFromFormat(PixelFormat mFormat);

		/**	Returns pixel format closest to the provided internal OpenGL format. */
		static PixelFormat getClosestEngineFormat(GLenum fmt);

		/**	Returns closest supported format to the provided format. */
		static PixelFormat getClosestValidFormat(PixelFormat fmt);

		/**	Converts an OpenGL internal format into base format. */
		static GLenum getBaseFormatFromCompressedInternalFormat(GLenum internalFormat);

		/** Converts engine GPU buffer format to OpenGL GPU buffer format. */
		static GLenum getBufferFormat(GpuBufferFormat format);

	};

	/** @} */
};
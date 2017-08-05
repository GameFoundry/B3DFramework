//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLPixelFormat.h"
#include "BsRenderAPI.h"
#include "BsBitwise.h"
#include "BsDebug.h"

namespace bs { namespace ct
{
	PixelFormat GLPixelUtil::getClosestSupportedPF(PixelFormat pf, TextureType texType, int usage)
	{
		// Check for any obvious issues first
		PixelUtil::checkFormat(pf, texType, usage);
		
		// We don't check for any platform-specific format issues, assumed all are supported

		return pf;
	}

	GLenum GLPixelUtil::getGLOriginFormat(PixelFormat mFormat)
	{
		switch (mFormat)
		{
		case PF_R8:
		case PF_R8I:
		case PF_R8S:
			return GL_RED;
		case PF_RG8:
		case PF_RG8I:
		case PF_RG8S:
			return GL_RG;
		case PF_RGB8:
			return GL_RGB;
		case PF_BGR8:
			return GL_BGR;
		case PF_RGBA8:
		case PF_RGBA8I:
		case PF_RGBA8S:
			return GL_RGBA;
		case PF_BGRA8:
			return GL_BGRA;
		case PF_R16F:
		case PF_R16I:
		case PF_R16S:
		case PF_R16:
			return GL_RED;
		case PF_RG16F:
		case PF_RG16I:
		case PF_RG16S:
		case PF_RG16:
			return GL_RG;
		case PF_RGBA16F:
		case PF_RGBA16I:
		case PF_RGBA16S:
		case PF_RGBA16:
			return GL_RGBA;
		case PF_R32F:
		case PF_R32I:
			return GL_RED;
		case PF_RG32F:
		case PF_RG32I:
			return GL_RG;
		case PF_RGB32F:
		case PF_RGB32I:
			return GL_RGB;
		case PF_RGBA32F:
		case PF_RGBA32I:
			return GL_RGBA;
		case PF_RG11B10F:
			return GL_RGB;
		case PF_RGB10A2:
			return GL_RGBA;
		case PF_BC1:
		case PF_BC1a:
		case PF_BC3:
		case PF_BC7:
			return GL_RGBA;
		case PF_BC4:
			return GL_RED;
		case PF_BC5:
			return GL_RG;
		case PF_BC6H:
			return GL_RGB;
		default:
			return 0;
		}
	}

	GLenum GLPixelUtil::getGLOriginDataType(PixelFormat format)
	{
		switch (format)
		{
		case PF_R8:
		case PF_RG8:
		case PF_RGB8:
		case PF_BGR8:
			return GL_UNSIGNED_BYTE;
		case PF_BGRA8:
		case PF_RGBA8:
			return GL_UNSIGNED_INT_8_8_8_8_REV;
		case PF_R8I:
		case PF_RG8I:
		case PF_RGBA8I:
		case PF_R8S:
		case PF_RG8S:
		case PF_RGBA8S:
			return GL_BYTE;
		case PF_R16I:
		case PF_RG16I:
		case PF_RGBA16I:
		case PF_R16S:
		case PF_RG16S:
		case PF_RGBA16S:
			return GL_SHORT;
		case PF_R16:
		case PF_RG16:
		case PF_RGBA16:
			return GL_UNSIGNED_SHORT;
		case PF_R16F:
		case PF_RG16F:
		case PF_RGBA16F:
			return GL_HALF_FLOAT;
		case PF_R32I:
		case PF_RG32I:
		case PF_RGB32I:
		case PF_RGBA32I:
			return GL_INT;
		case PF_R32F:
		case PF_RG32F:
		case PF_RGB32F:
		case PF_RGBA32F:
			return GL_FLOAT;
		case PF_RG11B10F:
			return GL_UNSIGNED_INT_10F_11F_11F_REV;
		case PF_RGB10A2:
			return GL_UNSIGNED_INT_2_10_10_10_REV;
		default:
			return 0;
		}
	}

	GLenum GLPixelUtil::getGLInternalFormat(PixelFormat mFormat, bool hwGamma)
	{
		switch (mFormat) {
		case PF_R8:
			return GL_R8;
		case PF_R8I:
			return GL_R8I;
		case PF_R8S:
			return GL_R8_SNORM;
		case PF_RG8:
			return GL_RG8;
		case PF_RG8I:
			return GL_RG8I;
		case PF_RG8S:
			return GL_RG8_SNORM;
		case PF_RGB8:
		case PF_BGR8:
			if (hwGamma)
				return GL_SRGB8;
			else
				return GL_RGB8;
		case PF_BGRA8:
		case PF_RGBA8:
			if (hwGamma)
				return GL_SRGB8_ALPHA8;
			else
				return GL_RGBA8;
		case PF_RGBA8I:
			return GL_RGBA8I;
		case PF_RGBA8S:
			return GL_RGBA8_SNORM;
		case PF_R16F:
			return GL_R16F;
		case PF_R16I:
			return GL_R16I;
		case PF_R16S:
			return GL_R16_SNORM;
		case PF_R16:
			return GL_R16;
		case PF_RG16F:
			return GL_RG16F;
		case PF_RG16I:
			return GL_RG16I;
		case PF_RG16S:
			return GL_RG16_SNORM;
		case PF_RG16:
			return GL_RG16;
		case PF_RGBA16F:
			return GL_RGBA16F;
		case PF_RGBA16I:
			return GL_RGBA16I;
		case PF_RGBA16S:
			return GL_RGBA16_SNORM;
		case PF_RGBA16:
			return GL_RGBA16;
		case PF_R32F:
			return GL_R32F;
		case PF_R32I:
			return GL_R32I;
		case PF_RG32F:
			return GL_RG32F;
		case PF_RG32I:
			return GL_RG32I;
		case PF_RGB32F:
			return GL_RGB32F;
		case PF_RGB32I:
			return GL_RGB32I;
		case PF_RGBA32F:
			return GL_RGBA32F;
		case PF_RGBA32I:
			return GL_RGBA32I;
		case PF_BC1a:
		case PF_BC1:
			if (hwGamma)
				return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
			else
				return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		case PF_BC2:
			if (hwGamma)
				return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
			else
				return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		case PF_BC3:
			if (hwGamma)
				return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			else
				return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case PF_BC4:
			return GL_COMPRESSED_RED_RGTC1;
		case PF_BC5:
			return GL_COMPRESSED_RG_RGTC2;
		case PF_BC6H:
			return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
		case PF_BC7:
			if (hwGamma)
				return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
			else
				return GL_COMPRESSED_RGBA_BPTC_UNORM;
		case PF_D16:
			return GL_DEPTH_COMPONENT16;
		case PF_D32:
			return GL_DEPTH_COMPONENT32F;
		case PF_D24S8:
			return GL_DEPTH24_STENCIL8;
		case PF_D32_S8X24:
			return GL_DEPTH32F_STENCIL8;
		case PF_RG11B10F:
			return GL_R11F_G11F_B10F;
		case PF_RGB10A2:
			return GL_RGB10_A2;
		default:
			return GL_NONE;
		}
	}

	GLenum GLPixelUtil::getDepthStencilTypeFromPF(PixelFormat mFormat)
	{
		switch(mFormat)
		{
		case PF_D32_S8X24:
			return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		case PF_D24S8:
			return GL_UNSIGNED_INT_24_8;
		case PF_D32:
			return GL_FLOAT;
		case PF_D16:
			return GL_UNSIGNED_SHORT;
		default:
			break;
		}

		LOGERR("Invalid depth stencil format");
		return PF_D32_S8X24;
	}
		
	GLenum GLPixelUtil::getDepthStencilFormatFromPF(PixelFormat mFormat)
	{
		switch (mFormat)
		{
		case PF_D32_S8X24:
			return GL_DEPTH_STENCIL;
		case PF_D24S8:
			return GL_DEPTH_STENCIL;
		case PF_D32:
			return GL_DEPTH_COMPONENT;
		case PF_D16:
			return GL_DEPTH_COMPONENT;
		default:
			break;
		}

		LOGERR("Invalid depth stencil format");
		return GL_DEPTH_STENCIL;
	}

	GLenum GLPixelUtil::getBufferFormat(GpuBufferFormat format)
	{
		static bool lookupInitialized = false;

		static GLenum lookup[BF_COUNT];
		if (!lookupInitialized)
		{
			lookup[BF_16X1F] = GL_R16F;
			lookup[BF_16X2F] = GL_RG16F;
			lookup[BF_16X4F] = GL_RGBA16F;
			lookup[BF_32X1F] = GL_R32F;
			lookup[BF_32X2F] = GL_RG32F;
			lookup[BF_32X3F] = GL_RGB32F;
			lookup[BF_32X4F] = GL_RGBA32F;
			lookup[BF_8X1] = GL_R8;
			lookup[BF_8X2] = GL_RG8;
			lookup[BF_8X4] = GL_RGBA8;
			lookup[BF_16X1] = GL_R16;
			lookup[BF_16X2] = GL_RG16;
			lookup[BF_16X4] = GL_RGBA16;
			lookup[BF_8X1S] = GL_R8I;
			lookup[BF_8X2S] = GL_RG8I;
			lookup[BF_8X4S] = GL_RGBA8I;
			lookup[BF_16X1S] = GL_R16I;
			lookup[BF_16X2S] = GL_RG16I;
			lookup[BF_16X4S] = GL_RGBA16I;
			lookup[BF_32X1S] = GL_R32I;
			lookup[BF_32X2S] = GL_RG32I;
			lookup[BF_32X3S] = GL_RGB32I;
			lookup[BF_32X4S] = GL_RGBA32I;
			lookup[BF_8X1U] = GL_R8UI;
			lookup[BF_8X2U] = GL_RG8UI;
			lookup[BF_8X4U] = GL_RGBA8UI;
			lookup[BF_16X1U] = GL_R16UI;
			lookup[BF_16X2U] = GL_RG16UI;
			lookup[BF_16X4U] = GL_RGBA16UI;
			lookup[BF_32X1U] = GL_R32UI;
			lookup[BF_32X2U] = GL_RG32UI;
			lookup[BF_32X3U] = GL_RGB32UI;
			lookup[BF_32X4U] = GL_RGBA32UI;

			lookupInitialized = true;
		}

		if (format >= BF_COUNT)
			return GL_NONE;

		return lookup[(UINT32)format];
	}
}}
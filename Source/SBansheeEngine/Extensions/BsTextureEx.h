//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Image/BsPixelData.h"
#include "Image/BsTexture.h"

namespace bs
{
	class AsyncOpEx;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	 /** Extension class for Texture, for adding additional functionality for the script version of PixelData. */
	class BS_SCRIPT_EXPORT(e:Texture) TextureEx
	{
	public:
		BS_SCRIPT_EXPORT(ec:Texture,v:private)
		static HTexture create(PixelFormat format, UINT32 width, UINT32 height, UINT32 depth, TextureType texType, 
			TextureUsage usage, UINT32 numSamples, bool hasMipmaps, bool gammaCorrection);

		/** @copydoc TextureProperties::getFormat */
		BS_SCRIPT_EXPORT(e:Texture,n:PixelFormat,pr:getter)
		static PixelFormat getPixelFormat(const HTexture& thisPtr);

		/** @copydoc TextureProperties::getUsage */
		BS_SCRIPT_EXPORT(e:Texture,n:Usage,pr:getter)
		static TextureUsage getUsage(const HTexture& thisPtr);

		/** @copydoc TextureProperties::getTextureType */
		BS_SCRIPT_EXPORT(e:Texture,n:Type,pr:getter)
		static TextureType getType(const HTexture& thisPtr);

		/** @copydoc TextureProperties::getWidth */
		BS_SCRIPT_EXPORT(e:Texture,n:Width,pr:getter)
		static UINT32 getWidth(const HTexture& thisPtr);

		/** @copydoc TextureProperties::getHeight */
		BS_SCRIPT_EXPORT(e:Texture,n:Height,pr:getter)
		static UINT32 getHeight(const HTexture& thisPtr);

		/** @copydoc TextureProperties::getDepth */
		BS_SCRIPT_EXPORT(e:Texture,n:Depth,pr:getter)
		static UINT32 getDepth(const HTexture& thisPtr);

		/** @copydoc TextureProperties::isHardwareGammaEnabled */
		BS_SCRIPT_EXPORT(e:Texture,n:GammaSpace,pr:getter)
		static bool getGammaCorrection(const HTexture& thisPtr);

		/** @copydoc TextureProperties::getNumSamples */
		BS_SCRIPT_EXPORT(e:Texture,n:SampleCount,pr:getter)
		static UINT32 getSampleCount(const HTexture& thisPtr);

		/** @copydoc TextureProperties::getNumMipmaps */
		BS_SCRIPT_EXPORT(e:Texture,n:MipMapCount,pr:getter)
		static UINT32 getMipmapCount(const HTexture& thisPtr);

		/**
		 * Returns pixels for the specified mip level & face. Pixels will be read from system memory, which means the
		 * texture has to be created with TextureUsage.CPUCached. If the texture was updated from the GPU the
		 * pixels retrieved from this method will not reflect that, and you should use GetGPUPixels instead.
		 * 
		 * @param mipLevel	Mip level to retrieve pixels for. Top level (0) is the highest quality.
		 * @param face		Face to read the pixels from. Cubemap textures have six faces whose face indices are as
		 *					specified in the CubeFace enum. Array textures can have an arbitrary number of faces (if it's a
		 *					cubemap array it has to be a multiple of 6).
		 * @return			A set of pixels for the specified mip level.
		 */
		BS_SCRIPT_EXPORT(e:Texture,n:GetPixels)
		static SPtr<PixelData> getPixels(const HTexture& thisPtr, UINT32 face = 0, UINT32 mipLevel = 0);

		/**
		 * Reads texture pixels directly from the GPU. This is similar to GetPixels" but the texture doesn't
		 * need to be created with TextureUsage.CPUCached, and the data will contain any updates performed by
		 * the GPU. This method can be potentially slow as it introduces a CPU-GPU synchronization point. Additionally
		 * this method is asynchronous which means the data is not available immediately.
		 *
		 * @param mipLevel	Mip level to retrieve pixels for. Top level (0) is the highest quality.
		 * @param face		Face to read the pixels from. Cubemap textures have six faces whose face indices are as
		 *					specified in the CubeFace enum. Array textures can have an arbitrary number of faces (if it's a
		 *					cubemap array it has to be a multiple of 6).
		 * @return			AsyncOp object that will contain a PixelData object when the operation completes.
		 */
		BS_SCRIPT_EXPORT(e:Texture,n:GetGPUPixels)
		static SPtr<AsyncOpEx> getGPUPixels(const HTexture& thisPtr, UINT32 face = 0, UINT32 mipLevel = 0);

		/**
		 * Sets pixels for the specified mip level and face.
		 *
		 * @param data		Pixels to assign to the specified mip level. Pixel data must match the mip level size and
		 *					texture pixel format.
		 * @param mipLevel	Mip level to set pixels for. Top level (0) is the highest quality.
		 * @param face		Face to write the pixels to. Cubemap textures have six faces whose face indices are as
		 *					specified in the CubeFace enum. Array textures can have an arbitrary number of faces (if it's a
		 *					cubemap array it has to be a multiple of 6).
		 */
		BS_SCRIPT_EXPORT(e:Texture,n:SetPixels)
		static void setPixels(const HTexture& thisPtr, const SPtr<PixelData>& data, UINT32 face = 0, UINT32 mipLevel = 0);

		/**
		 * Sets pixels for the specified mip level and face.
		 *
		 * @param colors	Pixels to assign to the specified mip level. Size of the array must match the mip level
		 *                  dimensions. Data is expected to be laid out row by row. Pixels will be automatically
		 *                  converted to the valid pixel format.
		 * @param mipLevel	Mip level to set pixels for. Top level (0) is the highest quality.
		 * @param face		Face to write the pixels to. Cubemap textures have six faces whose face indices are as
		 *					specified in the CubeFace enum. Array textures can have an arbitrary number of faces (if it's a
		 *					cubemap array it has to be a multiple of 6).
		 */
		BS_SCRIPT_EXPORT(e:Texture,n:SetPixels)
		static void setPixelsArray(const HTexture& thisPtr, const Vector<Color>& colors, UINT32 face = 0, UINT32 mipLevel = 0);
	};

	/** @endcond */
	/** @} */
}

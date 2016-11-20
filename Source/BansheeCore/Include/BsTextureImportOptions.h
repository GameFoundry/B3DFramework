//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsImportOptions.h"
#include "BsPixelUtil.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Contains import options you may use to control how is a texture imported. */
	class BS_CORE_EXPORT TextureImportOptions : public ImportOptions
	{
	public:
		TextureImportOptions();

		/** Sets the pixel format to import as. */
		void setFormat(PixelFormat format) { mFormat = format; }

		/** Enables or disables mipmap generation for the texture. */
		void setGenerateMipmaps(bool generate) { mGenerateMips = generate; }

		/**
		 * Sets the maximum mip level to generate when generating mipmaps. If 0 then maximum amount of mip levels will be 
		 * generated.
		 */
		void setMaxMip(UINT32 maxMip) { mMaxMip = maxMip; }

		/** Sets whether the texture data is also stored in main memory, available for fast CPU access. */
		void setCPUCached(bool cached) { mCPUCached = cached; }

		/** Sets whether the texture data can be read directly from the GPU. */
		void setCPUReadable(bool readable) { mCPUReadable = readable; }

		/** 
		 * Sets whether the texture data should be treated as if its in sRGB (gamma) space. Such texture will be converted 
		 * by hardware to linear space before use on the GPU.
		 */
		void setSRGB(bool sRGB) { mSRGB = sRGB; }

		/** Gets the pixel format to import as. */
		PixelFormat getFormat() const { return mFormat; }

		/** Checks will be imported texture have automatically generated mipmaps. */
		bool getGenerateMipmaps() const { return mGenerateMips; }

		/**
		 * Gets the maximum mip level to generate when generating mipmaps. If 0 then maximum amount of mip levels will be 
		 * generated.
		 */
		UINT32 getMaxMip() const { return mMaxMip; }

		/** Retrieves whether the texture data is also stored in main memory, available for fast CPU access. */
		bool getCPUCached() const { return mCPUCached; }

		/** Retrieves whether the texture data can be read directly from the GPU. */
		bool getCPUReadable() const { return mCPUReadable; }

		/**
		 * Retrieves whether the texture data should be treated as if its in sRGB (gamma) space. Such texture will be 
		 * converted by hardware to linear space before use on the GPU.
		 */
		bool getSRGB() const { return mSRGB; }

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class TextureImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	private:
		PixelFormat mFormat;
		bool mGenerateMips;
		UINT32 mMaxMip;
		bool mCPUReadable;
		bool mCPUCached;
		bool mSRGB;
	};

	/** @} */
}
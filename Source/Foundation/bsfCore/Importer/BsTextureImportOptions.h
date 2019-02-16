//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Importer/BsImportOptions.h"
#include "Image/BsPixelUtil.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Contains import options you may use to control how is a texture imported. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Importer,api:bsf,api:bed) TextureImportOptions : public ImportOptions
	{
	public:
		TextureImportOptions() = default;

		/** Pixel format to import as. */
		BS_SCRIPT_EXPORT()
		PixelFormat format = PF_RGBA8;

		/** Enables or disables mipmap generation for the texture. */
		BS_SCRIPT_EXPORT()
		bool generateMips = false;

		/**
		 * Maximum mip level to generate when generating mipmaps. If 0 then maximum amount of mip levels will be generated.
		 */
		BS_SCRIPT_EXPORT()
		UINT32 maxMip = 0;

		/** Determines whether the texture data is also stored in main memory, available for fast CPU access. */
		BS_SCRIPT_EXPORT()
		bool cpuCached = false;

		/** 
		 * Determines whether the texture data should be treated as if its in sRGB (gamma) space. Such texture will be 
		 * converted by hardware to linear space before use on the GPU.
		 */
		BS_SCRIPT_EXPORT()
		bool sRGB = false;

		/** 
		 * Determines should the texture be imported as a cubemap. See setCubemapSource to choose how will the source
		 * texture be converted to a cubemap.
		 */
		BS_SCRIPT_EXPORT()
		bool cubemap = false;

		/** 
		 * Determines how should the source texture be interpreted when generating a cubemap. Only relevant when @p cubemap
		 * is set to true.
		 */
		BS_SCRIPT_EXPORT()
		CubemapSourceType cubemapSourceType = CubemapSourceType::Faces;

		/** Creates a new import options object that allows you to customize how are textures imported. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<TextureImportOptions> create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class TextureImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
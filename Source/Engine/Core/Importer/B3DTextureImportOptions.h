//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Importer/B3DImportOptions.h"
#include "Image/B3DPixelUtility.h"

namespace b3d
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Contains import options you may use to control how is a texture imported. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), API(Framework), API(Editor)) TextureImportOptions : public ImportOptions
	{
	public:
		TextureImportOptions() = default;

		/** Pixel format to import as. */
		B3D_SCRIPT_EXPORT()
		PixelFormat Format = PF_RGBA8;

		/** Enables or disables mipmap generation for the texture. */
		B3D_SCRIPT_EXPORT()
		bool GenerateMips = false;

		/**
		 * Maximum mip level to generate when generating mipmaps. If 0 then maximum amount of mip levels will be generated.
		 */
		B3D_SCRIPT_EXPORT()
		u32 MaxMip = 0;

		/** Determines whether the texture data is also stored in main memory, available for fast CPU access. */
		B3D_SCRIPT_EXPORT()
		bool CpuCached = false;

		/**
		 * Determines whether the texture data should be treated as if its in sRGB (gamma) space. Such texture will be
		 * converted by hardware to linear space before use on the GPU.
		 */
		B3D_SCRIPT_EXPORT()
		bool SRgb = false;

		/**
		 * Determines should the texture be imported as a cubemap. See CubemapSourceType to choose how will the source
		 * texture be converted to a cubemap.
		 */
		B3D_SCRIPT_EXPORT()
		bool Cubemap = false;

		/**
		 * Determines how should the source texture be interpreted when generating a cubemap. Only relevant when @p Cubemap
		 * is set to true.
		 */
		B3D_SCRIPT_EXPORT()
		CubemapSourceType CubemapSourceType = CubemapSourceType::Faces;

		/** Creates a new import options object that allows you to customize how are textures imported. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static TShared<TextureImportOptions> Create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class TextureImportOptionsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d

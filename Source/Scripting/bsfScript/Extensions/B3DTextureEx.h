//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DTexture.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for Texture, for adding additional functionality for the script version of PixelData. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(Texture)) TextureEx
	{
	public:
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Texture), Visibility(Private))
		static HTexture Create(PixelFormat format, u32 width, u32 height, u32 depth, TextureType texType, TextureUsageFlags usage, u32 numSamples, bool hasMipmaps, bool gammaCorrection);

		/** @copydoc TextureProperties::GetFormat */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(PixelFormat), Property(Getter))
		static PixelFormat GetPixelFormat(const HTexture& thisPtr);

		/** @copydoc TextureProperties::GetUsage */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(Usage), Property(Getter))
		static TextureUsageFlags GetUsage(const HTexture& thisPtr);

		/** @copydoc TextureProperties::GetTextureType */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(Type), Property(Getter))
		static TextureType GetType(const HTexture& thisPtr);

		/** @copydoc TextureProperties::GetWidth */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(Width), Property(Getter))
		static u32 GetWidth(const HTexture& thisPtr);

		/** @copydoc TextureProperties::GetHeight */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(Height), Property(Getter))
		static u32 GetHeight(const HTexture& thisPtr);

		/** @copydoc TextureProperties::GetDepth */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(Depth), Property(Getter))
		static u32 GetDepth(const HTexture& thisPtr);

		/** @copydoc TextureProperties::IsHardwareGammaEnabled */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(GammaSpace), Property(Getter))
		static bool GetGammaCorrection(const HTexture& thisPtr);

		/** @copydoc TextureProperties::GetNumSamples */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(SampleCount), Property(Getter))
		static u32 GetSampleCount(const HTexture& thisPtr);

		/** @copydoc TextureProperties::GetNumMipmaps */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(MipMapCount), Property(Getter))
		static u32 GetMipmapCount(const HTexture& thisPtr);

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
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(GetPixels))
		static TShared<PixelData> GetPixels(const HTexture& thisPtr, u32 face = 0, u32 mipLevel = 0);

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
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(SetPixels))
		static void SetPixels(const HTexture& thisPtr, const TShared<PixelData>& data, u32 face = 0, u32 mipLevel = 0);

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
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Texture), ExportName(SetPixels))
		static void SetPixelsArray(const HTexture& thisPtr, const Vector<Color>& colors, u32 face = 0, u32 mipLevel = 0);
	};

	/** @endcond */
	/** @} */
} // namespace b3d

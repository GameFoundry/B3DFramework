//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalTextureManager.h"
#include "B3DMetalRenderTexture.h"
#include "B3DMetalUtility.h"
#include "Image/B3DPixelUtility.h"

namespace b3d
{
	PixelFormat MetalTextureManager::GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma)
	{
		PixelUtility::CheckFormat(format, ttype, usage);

		// Preserve depth/stencil semantics when Apple GPUs cannot expose the requested format;
		// three-component formats expand to a matching four-component representation.
		if (!render::IsMetalPixelFormatSupported(format, hwGamma))
		{
			if (usage.IsSet(TextureUsageFlag::DepthStencil))
				return PF_D32_S8X24;

			switch (format)
			{
			case PF_BGR8:		return PF_BGRA8;
			case PF_RGB32F:	return PF_RGBA32F;
			case PF_RGB32I:	return PF_RGBA32I;
			case PF_RGB32U:	return PF_RGBA32U;
			case PF_RGB16:		return PF_RGBA16;
			default:			return PF_RGBA8;
			}
		}

		return format;
	}

	TShared<RenderTexture> MetalTextureManager::CreateRenderTextureImpl(const RenderTextureCreateInformation& createInformation)
	{
		auto renderTexture = B3DMakeShared<MetalRenderTexture>(createInformation);
		return renderTexture;
	}

	namespace render
	{
		MetalTextureManager::MetalTextureManager(GpuDevice& gpuDevice)
			: TextureManager(gpuDevice)
		{ }

		TShared<RenderTexture> MetalTextureManager::CreateRenderTextureInternal(const RenderTextureCreateInformation& createInformation)
		{
			auto renderTexture = B3DMakeShared<MetalRenderTexture>(createInformation);
			return renderTexture;
		}
	} // namespace render
} // namespace b3d

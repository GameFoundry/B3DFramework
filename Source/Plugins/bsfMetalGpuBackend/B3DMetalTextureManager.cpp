//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalTextureManager.h"
#include "B3DMetalRenderTexture.h"

namespace b3d
{
	PixelFormat MetalTextureManager::GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma)
	{
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

//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullTextureManager.h"
#include "B3DNullRenderTexture.h"

namespace b3d
{
	PixelFormat NullTextureManager::GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma)
	{
		return format;
	}

	TShared<RenderTexture> NullTextureManager::CreateRenderTextureImpl(const RenderTextureCreateInformation& createInformation)
	{
		auto renderTexture = B3DMakeShared<NullRenderTexture>(createInformation);
		return renderTexture;
	}

	namespace render
	{
		NullTextureManager::NullTextureManager(GpuDevice& gpuDevice)
			: TextureManager(gpuDevice)
		{ }

		TShared<RenderTexture> NullTextureManager::CreateRenderTextureInternal(const RenderTextureCreateInformation& createInformation)
		{
			auto renderTexture = B3DMakeShared<NullRenderTexture>(createInformation);
			return renderTexture;
		}
	} // namespace render
} // namespace b3d

//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DD3D12TextureManager.h"

#include "B3DD3D12RenderTexture.h"
#include "B3DD3D12Utility.h"
#include "Image/B3DTexture.h"

using namespace b3d;

PixelFormat D3D12TextureManager::GetNativeFormat(TextureType textureType, PixelFormat format, TextureUsageFlags usage, bool hardwareGamma)
{
	PixelUtility::CheckFormat(format, textureType, usage);

	if(render::D3D12Utility::GetDXGIFormat(format) == DXGI_FORMAT_UNKNOWN)
		return PF_RGBA8;

	return format;
}

TShared<RenderTexture> D3D12TextureManager::CreateRenderTextureImpl(const RenderTextureCreateInformation& createInformation)
{
	D3D12RenderTexture* renderTexture = new(B3DAllocate<D3D12RenderTexture>()) D3D12RenderTexture(createInformation);

	return B3DMakeSharedFromExisting<D3D12RenderTexture>(renderTexture);
}

namespace b3d::render
{
	TShared<RenderTexture> D3D12TextureManager::CreateRenderTextureInternal(const RenderTextureCreateInformation& createInformation)
	{
		TShared<D3D12RenderTexture> renderTexture = B3DMakeShared<D3D12RenderTexture>(createInformation);
		renderTexture->SetShared(renderTexture);

		return renderTexture;
	}
} // namespace b3d::render

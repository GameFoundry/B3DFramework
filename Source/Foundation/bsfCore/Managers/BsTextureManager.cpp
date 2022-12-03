//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsTextureManager.h"
#include "Error/BsException.h"
#include "Image/BsPixelUtil.h"
#include "RenderAPI/BsRenderAPI.h"

using namespace bs;

SPtr<Texture> TextureManager::CreateTexture(const TextureCreateInformation& desc)
{
	Texture* tex = new(B3DAllocate<Texture>()) Texture(desc);
	SPtr<Texture> ret = B3DMakeCoreFromExisting<Texture>(tex);

	ret->SetThisPtrInternal(ret);
	ret->Initialize();

	return ret;
}

SPtr<Texture> TextureManager::CreateTexture(const TextureCreateInformation& desc, const SPtr<PixelData>& pixelData)
{
	Texture* tex = new(B3DAllocate<Texture>()) Texture(desc, pixelData);
	SPtr<Texture> ret = B3DMakeCoreFromExisting<Texture>(tex);

	ret->SetThisPtrInternal(ret);
	ret->Initialize();

	return ret;
}

SPtr<Texture> TextureManager::CreateEmptyInternal()
{
	Texture* tex = new(B3DAllocate<Texture>()) Texture();
	SPtr<Texture> texture = B3DMakeCoreFromExisting<Texture>(tex);
	texture->SetThisPtrInternal(texture);

	return texture;
}

SPtr<RenderTexture> TextureManager::CreateRenderTexture(const TextureCreateInformation& colorDesc, bool createDepth, PixelFormat depthStencilFormat)
{
	TextureCreateInformation textureDesc = colorDesc;
	textureDesc.Usage = TU_RENDERTARGET;
	textureDesc.MipMapCount = 0;

	HTexture texture = Texture::Create(textureDesc);

	HTexture depthStencil;
	if(createDepth)
	{
		textureDesc.Format = depthStencilFormat;
		textureDesc.UseHardwareSRGB = false;
		textureDesc.Usage = TU_DEPTHSTENCIL;

		depthStencil = Texture::Create(textureDesc);
	}

	RENDER_TEXTURE_DESC desc;
	desc.ColorSurfaces[0].Texture = texture;
	desc.ColorSurfaces[0].Face = 0;
	desc.ColorSurfaces[0].NumFaces = 1;
	desc.ColorSurfaces[0].MipLevel = 0;

	desc.DepthStencilSurface.Texture = depthStencil;
	desc.DepthStencilSurface.Face = 0;
	desc.DepthStencilSurface.NumFaces = 1;
	desc.DepthStencilSurface.MipLevel = 0;

	SPtr<RenderTexture> newRT = CreateRenderTexture(desc);

	return newRT;
}

SPtr<RenderTexture> TextureManager::CreateRenderTexture(const RENDER_TEXTURE_DESC& desc)
{
	SPtr<RenderTexture> newRT = CreateRenderTextureImpl(desc);
	newRT->SetThisPtrInternal(newRT);
	newRT->Initialize();

	return newRT;
}

namespace bs { namespace ct
{
void TextureManager::OnStartUp()
{
	TextureCreateInformation desc;
	desc.Type = TEX_TYPE_2D;
	desc.Width = 2;
	desc.Height = 2;
	desc.Format = PF_RGBA8;
	desc.Usage = TU_STATIC;

	// White built-in texture
	desc.Name = "Builtin White";
	SPtr<Texture> whiteTexture = CreateTexture(desc);

	SPtr<PixelData> whitePixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	whitePixelData->SetColorAt(Color::kWhite, 0, 0);
	whitePixelData->SetColorAt(Color::kWhite, 0, 1);
	whitePixelData->SetColorAt(Color::kWhite, 1, 0);
	whitePixelData->SetColorAt(Color::kWhite, 1, 1);

	whiteTexture->WriteData(*whitePixelData);
	Texture::kWhite = whiteTexture;

	// Black built-in texture
	desc.Name = "Builtin Black";
	SPtr<Texture> blackTexture = CreateTexture(desc);

	SPtr<PixelData> blackPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	blackPixelData->SetColorAt(Color::kZero, 0, 0);
	blackPixelData->SetColorAt(Color::kZero, 0, 1);
	blackPixelData->SetColorAt(Color::kZero, 1, 0);
	blackPixelData->SetColorAt(Color::kZero, 1, 1);

	blackTexture->WriteData(*blackPixelData);
	Texture::kBlack = blackTexture;

	// Normal (Y = Up) built-in texture
	desc.Name = "Builtin Normal";
	SPtr<Texture> normalTexture = CreateTexture(desc);
	SPtr<PixelData> normalPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);

	Color encodedNormal(0.5f, 0.5f, 1.0f);
	normalPixelData->SetColorAt(encodedNormal, 0, 0);
	normalPixelData->SetColorAt(encodedNormal, 0, 1);
	normalPixelData->SetColorAt(encodedNormal, 1, 0);
	normalPixelData->SetColorAt(encodedNormal, 1, 1);

	normalTexture->WriteData(*normalPixelData);
	Texture::kNormal = normalTexture;
}

void TextureManager::OnShutDown()
{
	// Need to make sure these are freed while still on the core thread
	Texture::kWhite = nullptr;
	Texture::kBlack = nullptr;
	Texture::kNormal = nullptr;
}

SPtr<Texture> TextureManager::CreateTexture(const TextureCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	SPtr<Texture> newTex = CreateTextureInternal(desc, nullptr, deviceMask);
	newTex->Initialize();

	return newTex;
}

SPtr<RenderTexture> TextureManager::CreateRenderTexture(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx)
{
	SPtr<RenderTexture> newRT = CreateRenderTextureInternal(desc, deviceIdx);
	newRT->Initialize();

	return newRT;
}
}}

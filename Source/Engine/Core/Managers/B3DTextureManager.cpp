//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DTextureManager.h"
#include "Image/B3DPixelUtility.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DGpuDevice.h"

using namespace b3d;

TShared<RenderTexture> TextureManager::CreateRenderTexture(const TextureCreateInformation& colorDesc, bool createDepth, PixelFormat depthStencilFormat)
{
	TextureCreateInformation textureDesc = colorDesc;
	textureDesc.Usage = TextureUsageFlag::RenderTarget;
	textureDesc.MipMapCount = 0;

	HTexture texture = Texture::Create(textureDesc);

	HTexture depthStencil;
	if(createDepth)
	{
		textureDesc.Format = depthStencilFormat;
		textureDesc.UseHardwareSRGB = false;
		textureDesc.Usage = TextureUsageFlag::DepthStencil;

		depthStencil = Texture::Create(textureDesc);
	}

	RenderTextureCreateInformation desc;
	desc.ColorSurfaces[0].Texture = texture;
	desc.ColorSurfaces[0].Face = 0;
	desc.ColorSurfaces[0].FaceCount = 1;
	desc.ColorSurfaces[0].MipLevel = 0;

	desc.DepthStencilSurface.Texture = depthStencil;
	desc.DepthStencilSurface.Face = 0;
	desc.DepthStencilSurface.FaceCount = 1;
	desc.DepthStencilSurface.MipLevel = 0;

	TShared<RenderTexture> newRT = CreateRenderTexture(desc);

	return newRT;
}

TShared<RenderTexture> TextureManager::CreateRenderTexture(const RenderTextureCreateInformation& desc)
{
	TShared<RenderTexture> newRT = CreateRenderTextureImpl(desc);
	newRT->SetShared(newRT);
	newRT->Initialize();

	return newRT;
}

namespace b3d { namespace render
{
void TextureManager::OnStartUp()
{
	TextureCreateInformation createInformation;
	createInformation.Type = TEX_TYPE_2D;
	createInformation.Width = 2;
	createInformation.Height = 2;
	createInformation.Format = PF_RGBA8;
	createInformation.Usage = TextureUsageFlag::Default;

	// White built-in texture
	createInformation.Name = "Builtin White";
	TShared<Texture> whiteTexture = mGpuDevice.CreateTexture(createInformation);

	TShared<PixelData> whitePixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	whitePixelData->SetColorAt(Color::kWhite, 0, 0);
	whitePixelData->SetColorAt(Color::kWhite, 0, 1);
	whitePixelData->SetColorAt(Color::kWhite, 1, 0);
	whitePixelData->SetColorAt(Color::kWhite, 1, 1);

	TextureUtility::Write(whiteTexture, *whitePixelData);
	Texture::kWhite = whiteTexture;

	// Black built-in texture
	createInformation.Name = "Builtin Black";
	TShared<Texture> blackTexture = mGpuDevice.CreateTexture(createInformation);

	TShared<PixelData> blackPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	blackPixelData->SetColorAt(Color::kZero, 0, 0);
	blackPixelData->SetColorAt(Color::kZero, 0, 1);
	blackPixelData->SetColorAt(Color::kZero, 1, 0);
	blackPixelData->SetColorAt(Color::kZero, 1, 1);

	TextureUtility::Write(blackTexture, *blackPixelData);
	Texture::kBlack = blackTexture;


	// Pink built-in texture
	createInformation.Name = "Builtin Pink";
	TShared<Texture> pinkTexture = mGpuDevice.CreateTexture(createInformation);

	TShared<PixelData> pinkPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	pinkPixelData->SetColorAt(Color::kPink, 0, 0);
	pinkPixelData->SetColorAt(Color::kPink, 0, 1);
	pinkPixelData->SetColorAt(Color::kPink, 1, 0);
	pinkPixelData->SetColorAt(Color::kPink, 1, 1);

	TextureUtility::Write(pinkTexture, *pinkPixelData);
	Texture::kPink = pinkTexture;

	// Normal (Y = Up) built-in texture
	createInformation.Name = "Builtin Normal";
	TShared<Texture> normalTexture = mGpuDevice.CreateTexture(createInformation);
	TShared<PixelData> normalPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);

	Color encodedNormal(0.5f, 0.5f, 1.0f);
	normalPixelData->SetColorAt(encodedNormal, 0, 0);
	normalPixelData->SetColorAt(encodedNormal, 0, 1);
	normalPixelData->SetColorAt(encodedNormal, 1, 0);
	normalPixelData->SetColorAt(encodedNormal, 1, 1);

	TextureUtility::Write(normalTexture, *normalPixelData);
	Texture::kNormal = normalTexture;
}

void TextureManager::OnShutDown()
{
	// Need to make sure these are freed while still on the render thread
	Texture::kWhite = nullptr;
	Texture::kBlack = nullptr;
	Texture::kPink = nullptr;
	Texture::kNormal = nullptr;
}

TShared<RenderTexture> TextureManager::CreateRenderTexture(const RenderTextureCreateInformation& desc)
{
	TShared<RenderTexture> newRT = CreateRenderTextureInternal(desc);
	newRT->Initialize();

	return newRT;
}
}}

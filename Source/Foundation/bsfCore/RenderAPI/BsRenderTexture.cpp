//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderTexture.h"
#include "Error/BsException.h"
#include "Image/BsTexture.h"
#include "Managers/BsTextureManager.h"
#include "Resources/BsResources.h"
#include "CoreThread/BsCoreThread.h"
#include <Private/RTTI/BsRenderTargetRTTI.h>

using namespace bs;

RenderTextureProperties::RenderTextureProperties(const RENDER_TEXTURE_DESC& desc, bool requiresFlipping)
{
	u32 firstIdx = (u32)-1;
	bool requiresHwGamma = false;
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		HTexture texture = desc.ColorSurfaces[i].Texture;

		if(!texture.IsLoaded())
			continue;

		if(firstIdx == (u32)-1)
			firstIdx = i;

		requiresHwGamma |= texture->GetProperties().UseHardwareSRGB;
	}

	if(firstIdx == (u32)-1)
	{
		HTexture texture = desc.DepthStencilSurface.Texture;
		if(texture.IsLoaded())
		{
			const TextureProperties& texProps = texture->GetProperties();
			Construct(&texProps, desc.DepthStencilSurface.NumFaces, desc.DepthStencilSurface.MipLevel, requiresFlipping, false);
		}
	}
	else
	{
		HTexture texture = desc.ColorSurfaces[firstIdx].Texture;

		const TextureProperties& texProps = texture->GetProperties();
		Construct(&texProps, desc.ColorSurfaces[firstIdx].NumFaces, desc.ColorSurfaces[firstIdx].MipLevel, requiresFlipping, requiresHwGamma);
	}
}

RenderTextureProperties::RenderTextureProperties(const ct::RENDER_TEXTURE_DESC& desc, bool requiresFlipping)
{
	u32 firstIdx = (u32)-1;
	bool requiresHwGamma = false;
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		SPtr<ct::Texture> texture = desc.ColorSurfaces[i].Texture;

		if(texture == nullptr)
			continue;

		if(firstIdx == (u32)-1)
			firstIdx = i;

		requiresHwGamma |= texture->GetProperties().UseHardwareSRGB;
	}

	if(firstIdx == (u32)-1)
	{
		SPtr<ct::Texture> texture = desc.DepthStencilSurface.Texture;
		if(texture != nullptr)
		{
			const TextureProperties& texProps = texture->GetProperties();
			Construct(&texProps, desc.DepthStencilSurface.NumFaces, desc.DepthStencilSurface.MipLevel, requiresFlipping, false);
		}
	}
	else
	{
		SPtr<ct::Texture> texture = desc.ColorSurfaces[firstIdx].Texture;

		const TextureProperties& texProps = texture->GetProperties();
		Construct(&texProps, desc.ColorSurfaces[firstIdx].NumFaces, desc.ColorSurfaces[firstIdx].MipLevel, requiresFlipping, requiresHwGamma);
	}
}

void RenderTextureProperties::Construct(const TextureProperties* textureProps, u32 numSlices, u32 mipLevel, bool requiresFlipping, bool hwGamma)
{
	if(textureProps != nullptr)
	{
		PixelUtil::GetSizeForMipLevel(textureProps->Width, textureProps->Height, textureProps->Depth, mipLevel, Width, Height, numSlices);

		numSlices *= numSlices;
		MultisampleCount = textureProps->SampleCount;
	}

	IsWindow = false;
	RequiresTextureFlipping = requiresFlipping;
	this->HwGamma = hwGamma;
}

SPtr<RenderTexture> RenderTexture::Create(const TextureCreateInformation& desc, bool createDepth, PixelFormat depthStencilFormat)
{
	return TextureManager::Instance().CreateRenderTexture(desc, createDepth, depthStencilFormat);
}

SPtr<RenderTexture> RenderTexture::Create(const RENDER_TEXTURE_DESC& desc)
{
	return TextureManager::Instance().CreateRenderTexture(desc);
}

SPtr<ct::RenderTexture> RenderTexture::GetCore() const
{
	return std::static_pointer_cast<ct::RenderTexture>(mCoreSpecific);
}

RenderTexture::RenderTexture(const RENDER_TEXTURE_DESC& desc)
	: mDesc(desc)
{
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		if(desc.ColorSurfaces[i].Texture != nullptr)
			mBindableColorTex[i] = desc.ColorSurfaces[i].Texture;
	}

	if(desc.DepthStencilSurface.Texture != nullptr)
		mBindableDepthStencilTex = desc.DepthStencilSurface.Texture;
}

SPtr<ct::CoreObject> RenderTexture::CreateCore() const
{
	ct::RENDER_TEXTURE_DESC coreDesc;

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		ct::RENDER_SURFACE_DESC surfaceDesc;
		if(mDesc.ColorSurfaces[i].Texture.IsLoaded())
			surfaceDesc.Texture = mDesc.ColorSurfaces[i].Texture->GetCore();

		surfaceDesc.Face = mDesc.ColorSurfaces[i].Face;
		surfaceDesc.NumFaces = mDesc.ColorSurfaces[i].NumFaces;
		surfaceDesc.MipLevel = mDesc.ColorSurfaces[i].MipLevel;

		coreDesc.ColorSurfaces[i] = surfaceDesc;
	}

	if(mDesc.DepthStencilSurface.Texture.IsLoaded())
		coreDesc.DepthStencilSurface.Texture = mDesc.DepthStencilSurface.Texture->GetCore();

	coreDesc.DepthStencilSurface.Face = mDesc.DepthStencilSurface.Face;
	coreDesc.DepthStencilSurface.NumFaces = mDesc.DepthStencilSurface.NumFaces;
	coreDesc.DepthStencilSurface.MipLevel = mDesc.DepthStencilSurface.MipLevel;

	return ct::TextureManager::Instance().CreateRenderTextureInternal(coreDesc);
}

CoreSyncData RenderTexture::SyncToCore(FrameAlloc* allocator)
{
	u32 size = sizeof(RenderTextureProperties);
	u8* buffer = allocator->Alloc(size);

	RenderTextureProperties& props = const_cast<RenderTextureProperties&>(GetProperties());

	memcpy(buffer, (void*)&props, size);
	return CoreSyncData(buffer, size);
}

const RenderTextureProperties& RenderTexture::GetProperties() const
{
	return static_cast<const RenderTextureProperties&>(GetPropertiesInternal());
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTITypeBase* RenderTexture::GetRttiStatic()
{
	return RenderTextureRTTI::Instance();
}

RTTITypeBase* RenderTexture::GetRtti() const
{
	return RenderTexture::GetRttiStatic();
}

namespace bs { namespace ct
{
RenderTexture::RenderTexture(const RENDER_TEXTURE_DESC& desc)
	: mDesc(desc)
{}

void RenderTexture::Initialize()
{
	RenderTarget::Initialize();

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		if(mDesc.ColorSurfaces[i].Texture != nullptr)
		{
			SPtr<Texture> texture = mDesc.ColorSurfaces[i].Texture;

			if((texture->GetProperties().Usage & TU_RENDERTARGET) == 0)
				B3D_EXCEPT(InvalidParametersException, "Provided texture is not created with render target usage.");

			const TextureSurface textureSurface(mDesc.ColorSurfaces[i].MipLevel, 1, mDesc.ColorSurfaces[i].Face, mDesc.ColorSurfaces[i].NumFaces);
			mColorSurfaces[i] = texture->RequestView(textureSurface, GVU_RENDERTARGET);
		}
	}

	if(mDesc.DepthStencilSurface.Texture != nullptr)
	{
		SPtr<Texture> texture = mDesc.DepthStencilSurface.Texture;

		if((texture->GetProperties().Usage & TU_DEPTHSTENCIL) == 0)
			B3D_EXCEPT(InvalidParametersException, "Provided texture is not created with depth stencil usage.");

		const TextureSurface textureSurface(mDesc.DepthStencilSurface.MipLevel, 1, mDesc.DepthStencilSurface.Face, mDesc.DepthStencilSurface.NumFaces);
		mDepthStencilSurface = texture->RequestView(textureSurface, GVU_DEPTHSTENCIL);
	}

	ThrowIfBuffersDontMatch();
}

SPtr<RenderTexture> RenderTexture::Create(const RENDER_TEXTURE_DESC& desc)
{
	return TextureManager::Instance().CreateRenderTexture(desc);
}

void RenderTexture::SyncToCore(const CoreSyncData& data)
{
	RenderTextureProperties& props = const_cast<RenderTextureProperties&>(GetProperties());
	props = data.GetData<RenderTextureProperties>();
}

const RenderTextureProperties& RenderTexture::GetProperties() const
{
	return static_cast<const RenderTextureProperties&>(GetPropertiesInternal());
}

void RenderTexture::ThrowIfBuffersDontMatch() const
{
	u32 firstSurfaceIdx = (u32)-1;
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		if(mColorSurfaces[i] == nullptr)
			continue;

		if(firstSurfaceIdx == (u32)-1)
		{
			firstSurfaceIdx = i;
			continue;
		}

		const TextureProperties& curTexProps = mDesc.ColorSurfaces[i].Texture->GetProperties();
		const TextureProperties& firstTexProps = mDesc.ColorSurfaces[firstSurfaceIdx].Texture->GetProperties();

		u32 curMsCount = curTexProps.SampleCount;
		u32 firstMsCount = firstTexProps.SampleCount;

		u32 curNumSlices = mColorSurfaces[i]->GetInformation().Surface.FaceCount;
		u32 firstNumSlices = mColorSurfaces[firstSurfaceIdx]->GetInformation().Surface.FaceCount;

		if(curMsCount == 0)
			curMsCount = 1;

		if(firstMsCount == 0)
			firstMsCount = 1;

		if(curTexProps.Width != firstTexProps.Width ||
		   curTexProps.Height != firstTexProps.Height ||
		   curTexProps.Depth != firstTexProps.Depth ||
		   curMsCount != firstMsCount ||
		   curNumSlices != firstNumSlices)
		{
			String errorInfo = "\nWidth: " + ToString(curTexProps.Width) + "/" + ToString(firstTexProps.Width);
			errorInfo += "\nHeight: " + ToString(curTexProps.Height) + "/" + ToString(firstTexProps.Height);
			errorInfo += "\nDepth: " + ToString(curTexProps.Depth) + "/" + ToString(firstTexProps.Depth);
			errorInfo += "\nNum. slices: " + ToString(curNumSlices) + "/" + ToString(firstNumSlices);
			errorInfo += "\nMultisample Count: " + ToString(curMsCount) + "/" + ToString(firstMsCount);

			B3D_EXCEPT(InvalidParametersException, "Provided color textures don't match!" + errorInfo);
		}
	}

	if(firstSurfaceIdx != (u32)-1)
	{
		const TextureProperties& firstTexProps = mDesc.ColorSurfaces[firstSurfaceIdx].Texture->GetProperties();
		const SPtr<TextureView> firstSurfaceView = mColorSurfaces[firstSurfaceIdx];
		const TextureSurface& firstViewSurface = firstSurfaceView->GetInformation().Surface;

		u32 numSlices;
		if(firstTexProps.Type == TEX_TYPE_3D)
			numSlices = firstTexProps.Depth;
		else
			numSlices = firstTexProps.GetFaceCount();

		if((firstViewSurface.Face + firstViewSurface.FaceCount) > numSlices)
		{
			B3D_EXCEPT(InvalidParametersException, "Provided number of faces is out of range. Face: " + ToString(firstViewSurface.Face + firstViewSurface.FaceCount) + ". Max num faces: " + ToString(numSlices));
		}

		if(firstViewSurface.MipLevel > firstTexProps.MipMapCount)
		{
			B3D_EXCEPT(InvalidParametersException, "Provided number of mip maps is out of range. Mip level: " + ToString(firstViewSurface.MipLevel) + ". Max num mipmaps: " + ToString(firstTexProps.MipMapCount));
		}

		if(mDepthStencilSurface == nullptr)
			return;

		const TextureProperties& depthTexProps = mDesc.DepthStencilSurface.Texture->GetProperties();
		u32 depthMsCount = depthTexProps.SampleCount;
		u32 colorMsCount = firstTexProps.SampleCount;

		if(depthMsCount == 0)
			depthMsCount = 1;

		if(colorMsCount == 0)
			colorMsCount = 1;

		if(depthTexProps.Width != firstTexProps.Width ||
		   depthTexProps.Height != firstTexProps.Height ||
		   depthMsCount != colorMsCount)
		{
			String errorInfo = "\nWidth: " + ToString(depthTexProps.Width) + "/" + ToString(firstTexProps.Width);
			errorInfo += "\nHeight: " + ToString(depthTexProps.Height) + "/" + ToString(firstTexProps.Height);
			errorInfo += "\nMultisample Count: " + ToString(depthMsCount) + "/" + ToString(colorMsCount);

			B3D_EXCEPT(InvalidParametersException, "Provided texture and depth stencil buffer don't match!" + errorInfo);
		}
	}
}
}}

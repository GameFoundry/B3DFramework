//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DRenderTexture.h"
#include "Image/B3DTexture.h"
#include "Managers/B3DTextureManager.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "Resources/B3DResources.h"
#include "CoreObject/B3DRenderThread.h"
#include "RTTI/B3DRenderTargetRTTI.h"

#include "CoreObject/B3DCoreObjectSync.h"

using namespace b3d;

static RenderTargetProperties CreateRenderTextureProperties(const TextureProperties& textureProperties, u32 sliceCount, u32 mipLevel, bool requiresFlipping, bool hwGamma)
{
	RenderTargetProperties output;

	u32 depth;
	PixelUtility::GetSizeForMipLevel(textureProperties.Width, textureProperties.Height, textureProperties.Depth, mipLevel, output.Width, output.Height, depth);

	output.MultisampleCount = textureProperties.SampleCount;
	output.IsWindow = false;
	output.RequiresTextureFlipping = requiresFlipping;
	output.HwGamma = hwGamma;

	return output;
}

static RenderTargetProperties CreateRenderTextureProperties(const RenderTextureCreateInformation& createInformation, bool requiresFlipping)
{
	u32 firstIndex = ~0u;
	bool useHardwareSRGB = false;
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		HTexture texture = createInformation.ColorSurfaces[i].Texture;

		if(!texture.IsLoaded())
			continue;

		if(firstIndex == ~0u)
			firstIndex = i;

		useHardwareSRGB |= texture->GetProperties().UseHardwareSRGB;
	}

	if(firstIndex == ~0u)
	{
		HTexture texture = createInformation.DepthStencilSurface.Texture;
		if(texture.IsLoaded())
		{
			return CreateRenderTextureProperties(texture->GetProperties(), createInformation.DepthStencilSurface.FaceCount, createInformation.DepthStencilSurface.MipLevel, requiresFlipping, false);
		}
	}
	else
	{
		HTexture texture = createInformation.ColorSurfaces[firstIndex].Texture;

		return CreateRenderTextureProperties(texture->GetProperties(), createInformation.ColorSurfaces[firstIndex].FaceCount, createInformation.ColorSurfaces[firstIndex].MipLevel, requiresFlipping, useHardwareSRGB);
	}

	return RenderTargetProperties();
}

static RenderTargetProperties CreateRenderTextureProperties(const render::RenderTextureCreateInformation& createInformation, bool requiresFlipping)
{
	u32 firstIndex = ~0u;
	bool useHardwareSRGB = false;
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		SPtr<render::Texture> texture = createInformation.ColorSurfaces[i].Texture;

		if(texture == nullptr)
			continue;

		if(firstIndex == ~0u)
			firstIndex = i;

		useHardwareSRGB |= texture->GetProperties().UseHardwareSRGB;
	}

	if(firstIndex == ~0u)
	{
		SPtr<render::Texture> texture = createInformation.DepthStencilSurface.Texture;
		if(texture != nullptr)
		{
			return CreateRenderTextureProperties(texture->GetProperties(), createInformation.DepthStencilSurface.FaceCount, createInformation.DepthStencilSurface.MipLevel, requiresFlipping, false);
		}
	}
	else
	{
		SPtr<render::Texture> texture = createInformation.ColorSurfaces[firstIndex].Texture;

		return CreateRenderTextureProperties(texture->GetProperties(), createInformation.ColorSurfaces[firstIndex].FaceCount, createInformation.ColorSurfaces[firstIndex].MipLevel, requiresFlipping, useHardwareSRGB);
	}

	return RenderTargetProperties();
}

SPtr<RenderTexture> RenderTexture::Create(const TextureCreateInformation& textureCreateInformation, bool createDepth, PixelFormat depthStencilFormat)
{
	return TextureManager::Instance().CreateRenderTexture(textureCreateInformation, createDepth, depthStencilFormat);
}

SPtr<RenderTexture> RenderTexture::Create(const RenderTextureCreateInformation& createInformation)
{
	return TextureManager::Instance().CreateRenderTexture(createInformation);
}

RenderTexture::RenderTexture(const RenderTextureCreateInformation& createInformation)
	: mInformation(createInformation)
{
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		if(createInformation.ColorSurfaces[i].Texture != nullptr)
			mBindableColorTex[i] = createInformation.ColorSurfaces[i].Texture;
	}

	if(createInformation.DepthStencilSurface.Texture != nullptr)
		mBindableDepthStencilTex = createInformation.DepthStencilSurface.Texture;

	mRenderTargetProperties = CreateRenderTextureProperties(createInformation, false);
}

SPtr<render::RenderProxy> RenderTexture::CreateRenderProxy() const
{
	render::RenderTextureCreateInformation renderProxyCreateInformation;

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		render::RenderSurfaceInformation surfaceInformation;
		surfaceInformation.Texture = B3DGetRenderProxy(mInformation.ColorSurfaces[i].Texture);
		surfaceInformation.Face = mInformation.ColorSurfaces[i].Face;
		surfaceInformation.FaceCount = mInformation.ColorSurfaces[i].FaceCount;
		surfaceInformation.MipLevel = mInformation.ColorSurfaces[i].MipLevel;

		renderProxyCreateInformation.ColorSurfaces[i] = surfaceInformation;
	}

	renderProxyCreateInformation.DepthStencilSurface.Texture = B3DGetRenderProxy(mInformation.DepthStencilSurface.Texture);
	renderProxyCreateInformation.DepthStencilSurface.Face = mInformation.DepthStencilSurface.Face;
	renderProxyCreateInformation.DepthStencilSurface.FaceCount = mInformation.DepthStencilSurface.FaceCount;
	renderProxyCreateInformation.DepthStencilSurface.MipLevel = mInformation.DepthStencilSurface.MipLevel;

	return render::TextureManager::Instance().CreateRenderTextureInternal(renderProxyCreateInformation);
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(RenderTexture, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mRenderTargetProperties)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* RenderTexture::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTIType* RenderTexture::GetRttiStatic()
{
	return RenderTextureRTTI::Instance();
}

RTTIType* RenderTexture::GetRtti() const
{
	return RenderTexture::GetRttiStatic();
}

namespace b3d { namespace render
{
RenderTexture::RenderTexture(const RenderTextureCreateInformation& createInformation)
	: mInformation(createInformation)
{
	mRenderTargetProperties = CreateRenderTextureProperties(createInformation, false);
}

void RenderTexture::Initialize()
{
	RenderTarget::Initialize();

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		if(mInformation.ColorSurfaces[i].Texture != nullptr)
		{
			SPtr<Texture> texture = mInformation.ColorSurfaces[i].Texture;

			B3D_ENSURE_LOG(texture->GetProperties().Usage.IsSet(TextureUsageFlag::RenderTarget), "Provided texture is not created with render target usage.");

			const TextureSurface textureSurface(mInformation.ColorSurfaces[i].MipLevel, 1, mInformation.ColorSurfaces[i].Face, mInformation.ColorSurfaces[i].FaceCount);
			mColorSurfaces[i] = texture->RequestView(textureSurface, GVU_RENDERTARGET);
		}
	}

	if(mInformation.DepthStencilSurface.Texture != nullptr)
	{
		SPtr<Texture> texture = mInformation.DepthStencilSurface.Texture;

		B3D_ENSURE_LOG(texture->GetProperties().Usage.IsSet(TextureUsageFlag::DepthStencil), "Provided texture is not created with depth stencil usage.");

		const TextureSurface textureSurface(mInformation.DepthStencilSurface.MipLevel, 1, mInformation.DepthStencilSurface.Face, mInformation.DepthStencilSurface.FaceCount);
		mDepthStencilSurface = texture->RequestView(textureSurface, GVU_DEPTHSTENCIL);
	}

	ReportIfBuffersDontMatch();
}

SPtr<RenderTexture> RenderTexture::Create(const RenderTextureCreateInformation& createInformation)
{
	return TextureManager::Instance().CreateRenderTexture(createInformation);
}

void RenderTexture::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::RenderTexture::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}

void RenderTexture::ReportIfBuffersDontMatch() const
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

		const TextureProperties& curTexProps = mInformation.ColorSurfaces[i].Texture->GetProperties();
		const TextureProperties& firstTexProps = mInformation.ColorSurfaces[firstSurfaceIdx].Texture->GetProperties();

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

			B3D_ENSURE_LOG(false, "Provided color textures don't match! {0}", errorInfo);
		}
	}

	if(firstSurfaceIdx != (u32)-1)
	{
		const TextureProperties& firstTexProps = mInformation.ColorSurfaces[firstSurfaceIdx].Texture->GetProperties();
		const SPtr<TextureView> firstSurfaceView = mColorSurfaces[firstSurfaceIdx];
		const TextureSurface& firstViewSurface = firstSurfaceView->GetInformation().Surface;

		u32 numSlices;
		if(firstTexProps.Type == TEX_TYPE_3D)
			numSlices = firstTexProps.Depth;
		else
			numSlices = firstTexProps.GetFaceCount();

		if((firstViewSurface.Face + firstViewSurface.FaceCount) > numSlices)
		{
			B3D_ENSURE_LOG(false, "Provided number of faces is out of range. Face: {0}. Max num faces: {1}", firstViewSurface.Face + firstViewSurface.FaceCount, numSlices);
			return;
		}

		if(firstViewSurface.MipLevel > firstTexProps.MipMapCount)
		{
			B3D_ENSURE_LOG(false, "Provided number of mip maps is out of range. Mip level: {0}. Max num mipmaps: {1}", firstViewSurface.MipLevel, firstTexProps.MipMapCount);
		}

		if(mDepthStencilSurface == nullptr)
			return;

		const TextureProperties& depthTexProps = mInformation.DepthStencilSurface.Texture->GetProperties();
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

			B3D_ENSURE_LOG(false, "Provided texture and depth stencil buffer don't match! {0}", errorInfo);
		}
	}
}

TAsyncOp<SPtr<PixelData>> RenderTexture::ReadAsync(GpuCommandBuffer& commandBuffer, u32 colorSurfaceIndex, u32 mipLevel, u32 arrayLayer)
{
	SPtr<Texture> colorTexture = GetColorTexture(colorSurfaceIndex);
	if(colorTexture == nullptr)
		return RenderTarget::ReadAsync(commandBuffer, colorSurfaceIndex, mipLevel, arrayLayer);

	return TextureUtility::ReadAsync(colorTexture, commandBuffer, mipLevel, arrayLayer);
}
}}

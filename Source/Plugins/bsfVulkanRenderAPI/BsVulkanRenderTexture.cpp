//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanRenderTexture.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanRenderPass.h"

using namespace bs;

VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc)
	: RenderTexture(desc), mProperties(desc, false)
{
}

namespace bs {
namespace ct {
VulkanRenderTexture::VulkanRenderTexture(VulkanGpuDevice& gpuDevice, const RENDER_TEXTURE_DESC& desc)
	: RenderTexture(desc), mGpuDevice(gpuDevice), mProperties(desc, false), mFramebuffer(nullptr)
{
}

void VulkanRenderTexture::Initialize()
{
	RenderTexture::Initialize();

	VulkanRenderPassCreateInformation renderPassInformation;
	renderPassInformation.SampleCount = mProperties.MultisampleCount > 1 ? mProperties.MultisampleCount : 1;
	renderPassInformation.IsOffscreenSurface = true;

	VulkanFramebufferInformation framebufferInformation;
	framebufferInformation.Width = mProperties.Width;
	framebufferInformation.Height = mProperties.Height;

	for(u32 renderTargetIndex = 0; renderTargetIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; ++renderTargetIndex)
	{
		if(mColorSurfaces[renderTargetIndex] == nullptr)
			continue;

		const SPtr<TextureView>& view = mColorSurfaces[renderTargetIndex];
		VulkanTexture* texture = static_cast<VulkanTexture*>(mDesc.ColorSurfaces[renderTargetIndex].Texture.get());

		VulkanImage* image = texture->GetVulkanResource();
		if(image == nullptr)
			continue;

		const TextureSurface& viewSurface = view->GetInformation().Surface;
		const u32 viewExplicitMipLevelCount = viewSurface.MipLevelCount == 0 ? (texture->GetProperties().MipMapCount + 1) : viewSurface.MipLevelCount;
		const u32 viewExplicitLayerCount = viewSurface.FaceCount == 0 ? texture->GetProperties().GetFaceCount() : viewSurface.FaceCount;

		TextureSurface surface;
		surface.MipLevel = viewSurface.MipLevel;
		surface.MipLevelCount = viewExplicitMipLevelCount;

		if(texture->GetProperties().Type == TEX_TYPE_3D)
		{
			if(viewSurface.Face > 0)
				B3D_LOG(Error, RenderBackend, "Non-zero array slice offset not supported when rendering to a 3D texture.");

			if(viewExplicitLayerCount > 1)
				B3D_LOG(Error, RenderBackend, "Cannot specify array slices when rendering to a 3D texture.");

			const u32 layerCount = texture->GetProperties().Depth;

			surface.Face = 0;
			surface.FaceCount = layerCount;

			framebufferInformation.Color[renderTargetIndex].BaseLayer = 0;
			framebufferInformation.Layers = layerCount;
		}
		else
		{
			surface.Face = viewSurface.Face;
			surface.FaceCount = viewExplicitLayerCount;

			framebufferInformation.Color[renderTargetIndex].BaseLayer = viewSurface.Face;
			framebufferInformation.Layers = viewExplicitLayerCount;
		}

		framebufferInformation.Color[renderTargetIndex].Image = image;
		framebufferInformation.Color[renderTargetIndex].Surface = surface;

		renderPassInformation.ColorAttachments[renderTargetIndex].IsEnabled = true;
		renderPassInformation.ColorAttachments[renderTargetIndex].IsShaderReadAllowed = image->IsShaderReadAllowed();

		const TextureProperties& textureProperties = texture->GetProperties();
		const PixelFormat internalFormat = texture->GetInternalFormat();

		renderPassInformation.ColorAttachments[renderTargetIndex].Format = VulkanUtility::GetPixelFormat(internalFormat, textureProperties.UseHardwareSRGB);
	}

	if(mDepthStencilSurface != nullptr)
	{
		const SPtr<TextureView>& view = mDepthStencilSurface;
		VulkanTexture* texture = static_cast<VulkanTexture*>(mDesc.DepthStencilSurface.Texture.get());

		VulkanImage* image = texture->GetVulkanResource();
		if(image != nullptr)
		{
			const TextureSurface& viewSurface = view->GetInformation().Surface;
			const u32 viewExplicitMipCount = viewSurface.MipLevelCount == 0 ? (texture->GetProperties().MipMapCount + 1) : viewSurface.MipLevelCount;
			const u32 viewExplicitLayerCount = viewSurface.FaceCount == 0 ? texture->GetProperties().GetFaceCount() : viewSurface.FaceCount;

			TextureSurface surface;
			surface.MipLevel = viewSurface.MipLevel;
			surface.MipLevelCount = viewExplicitMipCount;
			surface.Face = viewSurface.Face;

			if(texture->GetProperties().Type == TEX_TYPE_3D)
			{
				if(viewSurface.Face > 0)
					B3D_LOG(Error, RenderBackend, "Non-zero array slice offset not supported when rendering to a 3D texture.");

				if(viewExplicitLayerCount > 1)
					B3D_LOG(Error, RenderBackend, "Cannot specify array slices when rendering to a 3D texture.");

				surface.FaceCount = 1;
				framebufferInformation.Layers = 1;
			}
			else
			{
				surface.FaceCount = viewExplicitLayerCount;
				framebufferInformation.Layers = viewExplicitLayerCount;
			}

			framebufferInformation.Depth.Image = image;
			framebufferInformation.Depth.Surface = surface;
			framebufferInformation.Depth.BaseLayer = viewSurface.Face;

			renderPassInformation.DepthAttachment.IsEnabled = true;
			renderPassInformation.DepthAttachment.IsShaderReadAllowed = image->IsShaderReadAllowed();

			const TextureProperties& textureProperties = texture->GetProperties();
			const PixelFormat internalFormat = texture->GetInternalFormat();

			renderPassInformation.DepthAttachment.Format = VulkanUtility::GetPixelFormat(internalFormat, textureProperties.UseHardwareSRGB);
		}
	}

	mFramebuffer = VulkanFramebufferCache::Instance().FindOrCreateFramebuffer(mGpuDevice, framebufferInformation, renderPassInformation);
}

void VulkanRenderTexture::GetCustomAttribute(const String& name, void* data) const
{
	if(name == "FB")
	{
		VulkanFramebuffer** fb = (VulkanFramebuffer**)data;
		*fb = mFramebuffer;
		return;
	}
}
}} // namespace bs::ct

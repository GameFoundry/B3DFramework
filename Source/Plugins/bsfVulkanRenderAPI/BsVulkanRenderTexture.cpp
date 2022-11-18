//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanRenderTexture.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanRenderPass.h"

using namespace bs;

VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc)
	: RenderTexture(desc), mProperties(desc, false)
{
}

namespace bs {
namespace ct {
VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx)
	: RenderTexture(desc, deviceIdx), mProperties(desc, false), mDeviceIdx(deviceIdx), mFramebuffer(nullptr)
{
}

VulkanRenderTexture::~VulkanRenderTexture()
{
	mFramebuffer->Destroy();
}

void VulkanRenderTexture::Initialize()
{
	RenderTexture::Initialize();

	VulkanRenderPassCreateInformation rpDesc;
	rpDesc.SampleCount = mProperties.MultisampleCount > 1 ? mProperties.MultisampleCount : 1;
	rpDesc.IsOffscreenSurface = true;

	VULKAN_FRAMEBUFFER_DESC fbDesc;
	fbDesc.Width = mProperties.Width;
	fbDesc.Height = mProperties.Height;

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; ++i)
	{
		if(mColorSurfaces[i] == nullptr)
			continue;

		const SPtr<TextureView>& view = mColorSurfaces[i];
		VulkanTexture* texture = static_cast<VulkanTexture*>(mDesc.ColorSurfaces[i].Texture.get());

		VulkanImage* image = texture->GetResource(mDeviceIdx);
		if(image == nullptr)
			continue;

		const TextureSurface& viewSurface = view->GetInformation().Surface;
		const u32 viewExplicitMipLevelCount = viewSurface.MipLevelCount == 0 ? (texture->GetProperties().GetNumMipmaps() + 1) : viewSurface.MipLevelCount;
		const u32 viewExplicitLayerCount = viewSurface.FaceCount == 0 ? texture->GetProperties().GetNumFaces() : viewSurface.FaceCount;

		TextureSurface surface;
		surface.MipLevel = viewSurface.MipLevel;
		surface.MipLevelCount = viewExplicitMipLevelCount;

		if(texture->GetProperties().GetTextureType() == TEX_TYPE_3D)
		{
			if(viewSurface.Face > 0)
				B3D_LOG(Error, RenderBackend, "Non-zero array slice offset not supported when rendering to a 3D texture.");

			if(viewExplicitLayerCount > 1)
				B3D_LOG(Error, RenderBackend, "Cannot specify array slices when rendering to a 3D texture.");

			const u32 layerCount = texture->GetProperties().GetDepth();

			surface.Face = 0;
			surface.FaceCount = layerCount;

			fbDesc.Color[i].BaseLayer = 0;
			fbDesc.Layers = layerCount;
		}
		else
		{
			surface.Face = viewSurface.Face;
			surface.FaceCount = viewExplicitLayerCount;

			fbDesc.Color[i].BaseLayer = viewSurface.Face;
			fbDesc.Layers = viewExplicitLayerCount;
		}

		fbDesc.Color[i].Image = image;
		fbDesc.Color[i].Surface = surface;

		rpDesc.ColorAttachments[i].IsEnabled = true;
		rpDesc.ColorAttachments[i].IsShaderReadAllowed = image->IsShaderReadAllowed();
		rpDesc.ColorAttachments[i].Format = VulkanUtility::GetPixelFormat(texture->GetProperties().GetFormat(), texture->GetProperties().IsHardwareGammaEnabled());
	}

	if(mDepthStencilSurface != nullptr)
	{
		const SPtr<TextureView>& view = mDepthStencilSurface;
		VulkanTexture* texture = static_cast<VulkanTexture*>(mDesc.DepthStencilSurface.Texture.get());

		VulkanImage* image = texture->GetResource(mDeviceIdx);
		if(image != nullptr)
		{
			const TextureSurface& viewSurface = view->GetInformation().Surface;
			const u32 viewExplicitMipCount = viewSurface.MipLevelCount == 0 ? (texture->GetProperties().GetNumMipmaps() + 1) : viewSurface.MipLevelCount;
			const u32 viewExplicitLayerCount = viewSurface.FaceCount == 0 ? texture->GetProperties().GetNumFaces() : viewSurface.FaceCount;

			TextureSurface surface;
			surface.MipLevel = viewSurface.MipLevel;
			surface.MipLevelCount = viewExplicitMipCount;
			surface.Face = viewSurface.Face;

			if(texture->GetProperties().GetTextureType() == TEX_TYPE_3D)
			{
				if(viewSurface.Face > 0)
					B3D_LOG(Error, RenderBackend, "Non-zero array slice offset not supported when rendering to a 3D texture.");

				if(viewExplicitLayerCount > 1)
					B3D_LOG(Error, RenderBackend, "Cannot specify array slices when rendering to a 3D texture.");

				surface.FaceCount = 1;
				fbDesc.Layers = 1;
			}
			else
			{
				surface.FaceCount = viewExplicitLayerCount;
				fbDesc.Layers = viewExplicitLayerCount;
			}

			fbDesc.Depth.Image = image;
			fbDesc.Depth.Surface = surface;
			fbDesc.Depth.BaseLayer = viewSurface.Face;

			rpDesc.DepthAttachment.IsEnabled = true;
			rpDesc.DepthAttachment.IsShaderReadAllowed = image->IsShaderReadAllowed();
			rpDesc.DepthAttachment.Format = VulkanUtility::GetPixelFormat(texture->GetProperties().GetFormat(), texture->GetProperties().IsHardwareGammaEnabled());
		}
	}

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
	SPtr<VulkanDevice> device = rapi.GetDeviceInternal(mDeviceIdx);

	VulkanRenderPass* renderPass = VulkanRenderPasses::Instance().Get(device->GetLogical(), rpDesc);
	mFramebuffer = device->GetResourceManager().Create<VulkanFramebuffer>(renderPass, fbDesc);
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

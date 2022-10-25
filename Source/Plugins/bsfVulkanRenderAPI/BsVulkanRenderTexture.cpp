//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanRenderTexture.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanRenderPass.h"

namespace bs
{
	VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc)
		: RenderTexture(desc), mProperties(desc, false)
	{
	}

	namespace ct
	{
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
			Initialize();

			VULKAN_RENDER_PASS_DESC rpDesc;
			rpDesc.NumSamples = mProperties.MultisampleCount > 1 ? mProperties.MultisampleCount : 1;
			rpDesc.Offscreen = true;

			VULKAN_FRAMEBUFFER_DESC fbDesc;
			fbDesc.Width = mProperties.Width;
			fbDesc.Height = mProperties.Height;
			fbDesc.Layers = mProperties.NumSlices;

			for(u32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; ++i)
			{
				if(mColorSurfaces[i] == nullptr)
					continue;

				const SPtr<TextureView>& view = mColorSurfaces[i];
				VulkanTexture* texture = static_cast<VulkanTexture*>(mDesc.ColorSurfaces[i].Texture.get());

				VulkanImage* image = texture->GetResource(mDeviceIdx);
				if(image == nullptr)
					continue;

				TextureSurface surface;
				surface.MipLevel = view->GetMostDetailedMip();
				surface.NumMipLevels = view->GetNumMips();

				if(texture->GetProperties().GetTextureType() == TEX_TYPE_3D)
				{
					if(view->GetFirstArraySlice() > 0)
						BS_LOG(Error, RenderBackend, "Non-zero array slice offset not supported when rendering to a 3D texture.");

					if(view->GetNumArraySlices() > 1)
						BS_LOG(Error, RenderBackend, "Cannot specify array slices when rendering to a 3D texture.");

					surface.Face = 0;
					surface.NumFaces = mProperties.NumSlices;

					fbDesc.Color[i].BaseLayer = 0;
				}
				else
				{
					surface.Face = view->GetFirstArraySlice();
					surface.NumFaces = view->GetNumArraySlices();

					fbDesc.Color[i].BaseLayer = view->GetFirstArraySlice();
					fbDesc.Layers = view->GetNumArraySlices();
				}

				fbDesc.Color[i].Image = image;
				fbDesc.Color[i].Surface = surface;

				rpDesc.Color[i].Enabled = true;
				rpDesc.Color[i].Format = VulkanUtility::GetPixelFormat(texture->GetProperties().GetFormat(), texture->GetProperties().IsHardwareGammaEnabled());
			}

			if(mDepthStencilSurface != nullptr)
			{
				const SPtr<TextureView>& view = mDepthStencilSurface;
				VulkanTexture* texture = static_cast<VulkanTexture*>(mDesc.DepthStencilSurface.Texture.get());

				VulkanImage* image = texture->GetResource(mDeviceIdx);
				if(image != nullptr)
				{
					TextureSurface surface;
					surface.MipLevel = view->GetMostDetailedMip();
					surface.NumMipLevels = view->GetNumMips();

					if(texture->GetProperties().GetTextureType() == TEX_TYPE_3D)
					{
						if(view->GetFirstArraySlice() > 0)
							BS_LOG(Error, RenderBackend, "Non-zero array slice offset not supported when rendering to a 3D texture.");

						if(view->GetNumArraySlices() > 1)
							BS_LOG(Error, RenderBackend, "Cannot specify array slices when rendering to a 3D texture.");

						surface.Face = 0;
						surface.NumFaces = 1;
					}
					else
					{
						surface.Face = view->GetFirstArraySlice();
						surface.NumFaces = view->GetNumArraySlices();

						fbDesc.Layers = view->GetNumArraySlices();
					}

					fbDesc.Depth.Image = image;
					fbDesc.Depth.Surface = surface;
					fbDesc.Depth.BaseLayer = view->GetFirstArraySlice();

					rpDesc.Depth.Enabled = true;
					rpDesc.Depth.Format = VulkanUtility::GetPixelFormat(texture->GetProperties().GetFormat(), texture->GetProperties().IsHardwareGammaEnabled());
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
	} // namespace ct
} // namespace bs

//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanSamplerState.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"

namespace bs { namespace ct
{
	VulkanSampler::VulkanSampler(VulkanResourceManager* owner, VkSampler sampler)
		:VulkanResource(owner, true), mSampler(sampler)
	{ }

	VulkanSampler::~VulkanSampler()
	{
		vkDestroySampler(mOwner->GetDevice().GetLogical(), mSampler, gVulkanAllocator);
	}

	VulkanSamplerState::VulkanSamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask)
		:SamplerState(desc, deviceMask), mSamplers(), mDeviceMask(deviceMask)
	{ }

	VulkanSamplerState::~VulkanSamplerState()
	{
		for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mSamplers[i] == nullptr)
				return;

			mSamplers[i]->Destroy();
		}
	}

	void VulkanSamplerState::CreateInternal()
	{
		FilterOptions minFilter = getProperties().GetTextureFiltering(FT_MIN);
		FilterOptions magFilter = getProperties().GetTextureFiltering(FT_MAG);
		FilterOptions mipFilter = getProperties().GetTextureFiltering(FT_MIP);

		bool anisotropy = minFilter == FO_ANISOTROPIC || magFilter == FO_ANISOTROPIC || mipFilter == FO_ANISOTROPIC;
		const UVWAddressingMode& addressMode = getProperties().GetTextureAddressingMode();

		CompareFunction compareFunc = getProperties().GetComparisonFunction();

		VkSamplerCreateInfo samplerInfo;
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.flags = 0;
		samplerInfo.pNext = nullptr;
		samplerInfo.magFilter = VulkanUtility::getFilter(magFilter);
		samplerInfo.minFilter = VulkanUtility::getFilter(minFilter);
		samplerInfo.mipmapMode = VulkanUtility::getMipFilter(mipFilter);
		samplerInfo.addressModeU = VulkanUtility::getAddressingMode(addressMode.u);
		samplerInfo.addressModeV = VulkanUtility::getAddressingMode(addressMode.v);
		samplerInfo.addressModeW = VulkanUtility::getAddressingMode(addressMode.w);
		samplerInfo.mipLodBias = getProperties().GetTextureMipmapBias();
		samplerInfo.anisotropyEnable = anisotropy;
		samplerInfo.maxAnisotropy = (float)getProperties().GetTextureAnisotropy();
		samplerInfo.compareEnable = compareFunc != CMPF_ALWAYS_PASS;
		samplerInfo.compareOp = VulkanUtility::getCompareOp(compareFunc);
		samplerInfo.minLod = mProperties.GetMinimumMip();
		samplerInfo.maxLod = mProperties.GetMaximumMip();
		samplerInfo.borderColor = VulkanUtility::getBorderColor(getProperties().GetBorderColor());
		samplerInfo.unnormalizedCoordinates = false;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		// Allocate samplers per-device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
				break;

			VkSampler sampler;
			VkResult result = vkCreateSampler(devices[i]->GetLogical(), &samplerInfo, gVulkanAllocator, &sampler);
			assert(result == VK_SUCCESS);

			mSamplers[i] = devices[i]->GetResourceManager().create<VulkanSampler>(sampler);
		}
	}
}}

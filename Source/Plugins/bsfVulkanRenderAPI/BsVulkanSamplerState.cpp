//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanSamplerState.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"

using namespace bs;
using namespace bs::ct;

VulkanSampler::VulkanSampler(VulkanResourceManager* owner, VkSampler sampler)
	: VulkanResource(owner, true), mSampler(sampler)
{}

VulkanSampler::~VulkanSampler()
{
	vkDestroySampler(mOwner->GetDevice().GetLogical(), mSampler, gVulkanAllocator);
}

VulkanSamplerState::VulkanSamplerState(VulkanGpuDevice& gpuDevice, const SamplerStateInformation& desc)
	: SamplerState(desc), mGpuDevice(gpuDevice)
{}

VulkanSamplerState::~VulkanSamplerState()
{
	if(mSampler != nullptr)
		mSampler->Destroy();
}

void VulkanSamplerState::CreateInternal()
{
	FilterOptions minFilter = GetProperties().GetTextureFiltering(FT_MIN);
	FilterOptions magFilter = GetProperties().GetTextureFiltering(FT_MAG);
	FilterOptions mipFilter = GetProperties().GetTextureFiltering(FT_MIP);

	bool anisotropy = minFilter == FO_ANISOTROPIC || magFilter == FO_ANISOTROPIC || mipFilter == FO_ANISOTROPIC;
	const UVWAddressingMode& addressMode = GetProperties().GetTextureAddressingMode();

	CompareFunction compareFunc = GetProperties().GetComparisonFunction();

	VkSamplerCreateInfo samplerInfo;
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.flags = 0;
	samplerInfo.pNext = nullptr;
	samplerInfo.magFilter = VulkanUtility::GetFilter(magFilter);
	samplerInfo.minFilter = VulkanUtility::GetFilter(minFilter);
	samplerInfo.mipmapMode = VulkanUtility::GetMipFilter(mipFilter);
	samplerInfo.addressModeU = VulkanUtility::GetAddressingMode(addressMode.U);
	samplerInfo.addressModeV = VulkanUtility::GetAddressingMode(addressMode.V);
	samplerInfo.addressModeW = VulkanUtility::GetAddressingMode(addressMode.W);
	samplerInfo.mipLodBias = GetProperties().GetTextureMipmapBias();
	samplerInfo.anisotropyEnable = anisotropy;
	samplerInfo.maxAnisotropy = (float)GetProperties().GetTextureAnisotropy();
	samplerInfo.compareEnable = compareFunc != CMPF_ALWAYS_PASS;
	samplerInfo.compareOp = VulkanUtility::GetCompareOp(compareFunc);
	samplerInfo.minLod = mProperties.GetMinimumMip();
	samplerInfo.maxLod = mProperties.GetMaximumMip();
	samplerInfo.borderColor = VulkanUtility::GetBorderColor(GetProperties().GetBorderColor());
	samplerInfo.unnormalizedCoordinates = false;

	VkSampler sampler;
	VkResult result = vkCreateSampler(mGpuDevice.GetLogical(), &samplerInfo, gVulkanAllocator, &sampler);
	B3D_ASSERT(result == VK_SUCCESS);

	mSampler = mGpuDevice.GetResourceManager().Create<VulkanSampler>(sampler);
}

//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanSamplerState.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanUtility.h"

using namespace b3d;
using namespace b3d::render;

VulkanSampler::VulkanSampler(VulkanResourceManager* owner, VkSampler sampler, const StringView& name)
	: VulkanResource(owner, true, name), mSampler(sampler)
{}

VulkanSampler::~VulkanSampler()
{
	vkDestroySampler(mOwner->GetDevice().GetLogical(), mSampler, gVulkanAllocator);
}

VulkanSamplerState::VulkanSamplerState(VulkanGpuDevice& gpuDevice, const SamplerStateCreateInformation& createInformation)
	: SamplerState(createInformation), mGpuDevice(gpuDevice)
{}

VulkanSamplerState::~VulkanSamplerState()
{
	if(mSampler != nullptr)
		mSampler->Destroy();
}

void VulkanSamplerState::Initialize()
{
	FilterOptions minFilter = mInformation.MinFilter;
	FilterOptions magFilter = mInformation.MagFilter;
	FilterOptions mipFilter = mInformation.MipFilter;

	bool anisotropy = minFilter == FO_ANISOTROPIC || magFilter == FO_ANISOTROPIC || mipFilter == FO_ANISOTROPIC;
	const UVWAddressingMode& addressMode = mInformation.AddressMode;

	CompareFunction compareFunc = mInformation.ComparisonFunc;

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
	samplerInfo.mipLodBias = mInformation.MipmapBias;
	samplerInfo.anisotropyEnable = anisotropy;
	samplerInfo.maxAnisotropy = (float)mInformation.MaxAniso;
	samplerInfo.compareEnable = compareFunc != CMPF_ALWAYS_PASS;
	samplerInfo.compareOp = VulkanUtility::GetCompareOp(compareFunc);
	samplerInfo.minLod = mInformation.MipMin;
	samplerInfo.maxLod = mInformation.MipMax;
	samplerInfo.borderColor = VulkanUtility::GetBorderColor(mInformation.BorderColor);
	samplerInfo.unnormalizedCoordinates = false;

	VkSampler sampler;
	VkResult result = vkCreateSampler(mGpuDevice.GetLogical(), &samplerInfo, gVulkanAllocator, &sampler);
	B3D_ASSERT(result == VK_SUCCESS);

	mSampler = mGpuDevice.GetResourceManager().Create<VulkanSampler>(sampler);
}

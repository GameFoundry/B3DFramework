//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuParameterSetPool.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuParameterSet.h"
#include "B3DVulkanDescriptorSet.h"
#include "B3DVulkanGpuPipelineParameterLayout.h"

namespace b3d::render
{
	VulkanGpuParameterSetPool::VulkanGpuParameterSetPool(VulkanGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation)
		: GpuParameterSetPool(createInformation)
		, mDevice(device)
	{
		VkDescriptorPoolSize poolSizes[10];
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		poolSizes[0].descriptorCount = createInformation.MaxSampledImages;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		poolSizes[1].descriptorCount = createInformation.MaxSamplers;

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = createInformation.MaxCombinedImageSamplers;

		poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[3].descriptorCount = createInformation.MaxUniformBuffers;

		poolSizes[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[4].descriptorCount = createInformation.MaxUniformBuffersDynamic;

		poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[5].descriptorCount = createInformation.MaxStorageImages;

		poolSizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		poolSizes[6].descriptorCount = createInformation.MaxSampledBuffers;

		poolSizes[7].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		poolSizes[7].descriptorCount = createInformation.MaxStorageBuffers;

		poolSizes[8].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[8].descriptorCount = createInformation.MaxStorageBuffers;

		poolSizes[9].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		poolSizes[9].descriptorCount = createInformation.MaxStorageBuffersDynamic;

		VkDescriptorPoolCreateInfo poolCreateInformation;
		poolCreateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInformation.pNext = nullptr;
		poolCreateInformation.maxSets = createInformation.MaxSets;
		poolCreateInformation.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
		poolCreateInformation.pPoolSizes = poolSizes;

		// Transient mode: no individual free, enables O(1) reset
		// Persistent mode: individual free allowed, no bulk reset
		if (createInformation.Mode == GpuParameterSetPoolMode::Persistent)
			poolCreateInformation.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		else
			poolCreateInformation.flags = 0;

		VkResult result = vkCreateDescriptorPool(mDevice.GetLogical(), &poolCreateInformation, gVulkanAllocator, &mPool);
		B3D_ASSERT(result == VK_SUCCESS);
	}

	VulkanGpuParameterSetPool::~VulkanGpuParameterSetPool()
	{
		if (mPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(mDevice.GetLogical(), mPool, gVulkanAllocator);
	}

	SPtr<GpuParameterSet> VulkanGpuParameterSetPool::Create(const SPtr<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize)
	{
		SPtr<VulkanGpuParameterSet> output = B3DMakeShared<VulkanGpuParameterSet>(mDevice, layout, setIndex, *this);
		output->SetShared(output);

		if (!deferredInitialize)
			output->Initialize();

		return output;
	}

	void VulkanGpuParameterSetPool::Reset()
	{
		if (mInformation.Mode == GpuParameterSetPoolMode::Persistent)
		{
			B3D_LOG(Error, RenderBackend, "Cannot perform Reset on a Persistent mode parameter set pool.");
			return;
		}

#if B3D_BUILD_TYPE_DEVELOPMENT
		if (!mLiveDescriptorSets.empty())
		{
			B3D_LOG(Warning, RenderBackend, "Resetting parameter set pool with {0} live descriptor sets. "
				"These sets will become invalid.", mLiveDescriptorSets.size());
		}
		mLiveDescriptorSets.clear();
#endif

		VkResult result = vkResetDescriptorPool(mDevice.GetLogical(), mPool, 0);
		B3D_ASSERT(result == VK_SUCCESS);
	}

	VulkanDescriptorSet* VulkanGpuParameterSetPool::AllocateDescriptorSet(VkDescriptorSetLayout layout)
	{
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = mPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VkDescriptorSet vkSet = VK_NULL_HANDLE;
		VkResult result = vkAllocateDescriptorSets(mDevice.GetLogical(), &allocInfo, &vkSet);

		if (result != VK_SUCCESS)
		{
			B3D_LOG(Error, RenderBackend, "Failed to allocate descriptor set from pool.");
			return nullptr;
		}

		VulkanDescriptorSet* set = mDevice.GetResourceManager().Create<VulkanDescriptorSet>(vkSet, this);

#if B3D_BUILD_TYPE_DEVELOPMENT
		if(mInformation.Mode == GpuParameterSetPoolMode::Transient)
			mLiveDescriptorSets.insert(set);
#endif

		return set;
	}

	void VulkanGpuParameterSetPool::NotifyDescriptorSetDestroyed(VulkanDescriptorSet* set)
	{
		// Transient pools free all sets at once through Reset()
		if(mInformation.Mode == GpuParameterSetPoolMode::Transient)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			mLiveDescriptorSets.erase(set);
#endif
			return;
		}

		if (set == VK_NULL_HANDLE)
			return;

		VkDescriptorSet vkSet = set->GetVulkanHandle();
		VkResult result = vkFreeDescriptorSets(mDevice.GetLogical(), mPool, 1, &vkSet);
		B3D_ASSERT(result == VK_SUCCESS);
	}
} // namespace b3d::render

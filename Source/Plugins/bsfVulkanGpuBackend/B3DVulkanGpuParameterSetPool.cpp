//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuParameterSetPool.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuParameterSet.h"
#include "B3DVulkanDescriptorSet.h"
#include "B3DVulkanGpuPipelineParameterLayout.h"

namespace b3d::render
{
	VulkanGpuParameterSetPool::VulkanGpuParameterSetPool(VulkanGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation)
		: GpuParameterSetPool(createInformation) , mDevice(device)
	{
		mPools.push_back(CreateVkDescriptorPool());
	}

	VkDescriptorPool VulkanGpuParameterSetPool::CreateVkDescriptorPool() const
	{
		VkDescriptorPoolSize poolSizes[10];
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		poolSizes[0].descriptorCount = mInformation.MaxSampledImages;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		poolSizes[1].descriptorCount = mInformation.MaxSamplers;

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = mInformation.MaxCombinedImageSamplers;

		poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[3].descriptorCount = mInformation.MaxUniformBuffers;

		poolSizes[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[4].descriptorCount = mInformation.MaxUniformBuffersDynamic;

		poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[5].descriptorCount = mInformation.MaxStorageImages;

		poolSizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		poolSizes[6].descriptorCount = mInformation.MaxSampledBuffers;

		poolSizes[7].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		poolSizes[7].descriptorCount = mInformation.MaxStorageBuffers;

		poolSizes[8].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[8].descriptorCount = mInformation.MaxStorageBuffers;

		poolSizes[9].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		poolSizes[9].descriptorCount = mInformation.MaxStorageBuffersDynamic;

		VkDescriptorPoolCreateInfo poolCreateInformation;
		poolCreateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInformation.pNext = nullptr;
		poolCreateInformation.maxSets = mInformation.MaxSets;
		poolCreateInformation.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
		poolCreateInformation.pPoolSizes = poolSizes;

		// Transient mode: no individual free, enables O(1) reset
		// Persistent mode: individual free allowed, no bulk reset
		if (mInformation.Mode == GpuParameterSetPoolMode::Persistent)
			poolCreateInformation.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		else
			poolCreateInformation.flags = 0;

		VkDescriptorPool pool = VK_NULL_HANDLE;
		VkResult result = vkCreateDescriptorPool(mDevice.GetLogical(), &poolCreateInformation, gVulkanAllocator, &pool);
		B3D_ASSERT(result == VK_SUCCESS);

		return pool;
	}

	VulkanGpuParameterSetPool::~VulkanGpuParameterSetPool()
	{
		for (VkDescriptorPool pool : mPools)
		{
			if (pool != VK_NULL_HANDLE)
				vkDestroyDescriptorPool(mDevice.GetLogical(), pool, gVulkanAllocator);
		}
	}

	TShared<GpuParameterSet> VulkanGpuParameterSetPool::Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize)
	{
		TShared<VulkanGpuParameterSet> output = B3DMakeShared<VulkanGpuParameterSet>(mDevice, layout, setIndex, *this);
		output->SetShared(output);

		if (!deferredInitialize)
			output->Initialize();

		return output;
	}

	void VulkanGpuParameterSetPool::Reset()
	{
		if (mInformation.Mode == GpuParameterSetPoolMode::Persistent)
		{
			B3D_LOG(Error, LogRenderBackend, "Cannot perform Reset on a Persistent mode parameter set pool.");
			return;
		}

#if B3D_BUILD_TYPE_DEVELOPMENT
		if (!mLiveDescriptorSets.empty())
		{
			B3D_LOG(Warning, LogRenderBackend, "Resetting parameter set pool with {0} live descriptor sets. "
				"These sets will become invalid.", mLiveDescriptorSets.size());
		}
		mLiveDescriptorSets.clear();
#endif

		// Reset all internal pools
		for (VkDescriptorPool pool : mPools)
		{
			VkResult result = vkResetDescriptorPool(mDevice.GetLogical(), pool, 0);
			B3D_ASSERT(result == VK_SUCCESS);
		}
	}

	VulkanDescriptorSet* VulkanGpuParameterSetPool::AllocateDescriptorSet(VkDescriptorSetLayout layout)
	{
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		// Try to allocate from the last pool (most likely to have space)
		VkDescriptorPool currentPool = mPools.back();
		allocInfo.descriptorPool = currentPool;

		VkDescriptorSet vkSet = VK_NULL_HANDLE;
		VkResult result = vkAllocateDescriptorSets(mDevice.GetLogical(), &allocInfo, &vkSet);

		// If allocation failed due to pool exhaustion, create a new pool and retry
		if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
		{
			currentPool = CreateVkDescriptorPool();
			mPools.push_back(currentPool);

			allocInfo.descriptorPool = currentPool;
			result = vkAllocateDescriptorSets(mDevice.GetLogical(), &allocInfo, &vkSet);
		}

		if (result != VK_SUCCESS)
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to allocate descriptor set from pool.");
			return nullptr;
		}

		VulkanDescriptorSet* set = mDevice.GetResourceManager().Create<VulkanDescriptorSet>(vkSet, currentPool, this);

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

		if (set == nullptr)
			return;

		VkDescriptorSet vkSet = set->GetVulkanHandle();
		VkDescriptorPool vkDescriptorPool = set->GetVkDescriptorPool();
		VkResult result = vkFreeDescriptorSets(mDevice.GetLogical(), vkDescriptorPool, 1, &vkSet);
		B3D_ASSERT(result == VK_SUCCESS);
	}
} // namespace b3d::render
